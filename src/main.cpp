#include <raylib.h>
#include "vstd/vmath.h"
#include <math.h>
#include "entity.h"
#include "vstd/vrandom_gen.h"

constexpr i32 WIN_WIDTH = 800;

constexpr i32 WIN_HEIGHT = 800;
constexpr auto TITLE = "ROTATOS\n";
constexpr f32 PLAYER_MAX_SPEED = 15.0f;
constexpr f32 PLAYER_DASH_MAX_SPEED = 25.0f;
constexpr f32 PLAYER_DASH_LENGTH = 0.1f;
constexpr f32 PLAYER_DASH_COOLDOWN = PLAYER_DASH_LENGTH + 2.0f;
constexpr f32 PLAYER_ACCELERATION = 30.0f;

struct s_Game;

struct CameraState {
    enum State {
        FOLLOW,
        STATIC
    } state = STATIC;
    Camera2D camera = {};
    
    Camera2D* operator->() { return &camera; };
    operator Camera2D&() { return camera; };
}; 


struct Game {
    vec2 win_size = {};

    Entity player;
    EnemyCloud enemy_cloud;

    std::array<Bullet, NUM_BULLETS> bullets;
    size_t bullet_count = 0;

    CameraState camera_state = {};

    Game(i32 w_width, i32 w_height, i32 num_enemies): 
        win_size(w_width, w_height), enemy_cloud(num_enemies, 200, -1) {
        player.movement_kind = e_MovementKind::CIRCULAR;
        player.radius = E_RADIUS;
        player.color = E_COLOR;
        player.polar.rad = PI * 0.5f;
        player.polar.length = CENTER_RADIUS + E_RADIUS;

        camera_state->target = {0, 0};
        camera_state->offset.x = WIN_WIDTH * 0.5f;
        camera_state->offset.y = WIN_HEIGHT * 0.5f;
        camera_state->zoom = 1.0f;
    }

    void UpdatePlayer(f32 dt) {
        if (player.dashing) {
            player.dash_time -= dt;
        } else {
            if(player.accelerating)
                player.speed_polar.rad += PLAYER_ACCELERATION *  dt * (f32) player.direction;

            player.dash_cooldown -= dt;

            player.speed_polar.rad *= FLOOR_FRICTION;
            player.speed_polar.rad = Clampf32(player.speed_polar.rad, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
        }
        
        // Makes movement equal independent of polar length.
        f32 delta = (player.speed_polar.rad / player.polar.length);
        player.polar.rad += delta;
        player.polar.rad = fmodf(player.polar.rad, 2.0f * PI);
        if (player.polar.rad < 0) player.polar.rad += 2.0f * PI;


        if (player.dash_time <= 0) {
            player.dashing = false;
        }

    }

    void UpdateOthers(f32 dt) {
        for (auto& bullet: bullets) {
            bullet.UpdateBullet(dt);
        }
            
        enemy_cloud.UpdateEnemies(dt);
            
        // if (enemy_cloud.time_till_shoot <= 0) {
        //     i32 random_entity = rng::randi32(0, Row::ROW_0);
        //     for (int i = random_entity; i < enemy_cloud.enemies.size(); i++) {
        //         if (enemy_cloud.enemies[i].is_alive) {
        //             bullets[bullet_count++] = enemy_cloud.enemies[i].Shoot(e_MovementKind::OUTER, e_EntityDirection::IN, e_Team::BAD_GUYS);
        //             break;
        //         }
        //     }
        //     enemy_cloud.time_till_shoot = rng::randf32(0.4f, 2.0f);
        // } else {
        //     enemy_cloud.time_till_shoot -= dt;
        // }
    }

    void UpdateCamera(f32 dt) {
        fvec2Polar player_on_polar = player.polar;
        player_on_polar.length += 50.0f;

        fvec2 player_pos =  PolarToCartesian(player_on_polar.length, player_on_polar.rad);
        camera_state->target.x = player_pos.x;
        camera_state->target.y = player_pos.y;
        camera_state->rotation = (player_on_polar.rad * RAD2DEG) - 90.0f;
    }

    void Collisions(f32 dt){
        for (Bullet& bullet: bullets){
            if (!bullet.is_alive) continue;
            fvec2 bullet_pos = PolarToCartesian(bullet.polar.length, bullet.polar.rad);

            if (bullet.team == e_Team::GOOD_GUYS) {
                for (Entity& enemy: enemy_cloud.enemies) {
                    if (!enemy.is_alive) continue;
                    fvec2 enemy_pos = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                    if(CheckCollisionCircles(
                        {bullet_pos.x, bullet_pos.y}, bullet.radius, 
                        {enemy_pos.x, enemy_pos.y}, enemy.radius
                    )){
                        bullet.is_alive = false;
                        enemy.is_alive = false;
                        break;
                    }
                }
            } else if (bullet.team == e_Team::BAD_GUYS) {
                fvec2 player_pos = PolarToCartesian(player.polar.length, player.polar.rad);
                if(CheckCollisionCircles(
                    {bullet_pos.x, bullet_pos.y}, bullet.radius, 
                    {player_pos.x, player_pos.y}, player.radius
                )){
                    CloseWindow();
                }
            }
        }
    }

    void Draw() {
        ClearBackground(RAYWHITE);

        // World
        DrawCircle(0, 0, CENTER_RADIUS, RED);
        DrawCircleLines(0, 0, CENTER_RADIUS, BLACK);
        fvec2 player_pos = PolarToCartesian(player.polar.length, player.polar.rad);
        DrawCircle((i32)player_pos.x, (i32)player_pos.y, player.radius, player.color);

        for (const auto& bullet: bullets){
            if(bullet.is_alive){
                fvec2 bullet_pos = PolarToCartesian(bullet.polar.length, bullet.polar.rad);
                DrawCircle((i32)bullet_pos.x, (i32)bullet_pos.y, bullet.radius, bullet.color);
            }
        }
        for (const auto& enemy: enemy_cloud.enemies){
            if(enemy.is_alive){
                fvec2 enemy_pos = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                DrawCircle((i32)enemy_pos.x, (i32)enemy_pos.y, enemy.radius, enemy.color);
            }
        }
    }

    void ProcessInput(f32 dt) {
        // ZOOM
        f32 mouse_wheel = GetMouseWheelMove();   
        if(mouse_wheel != 0){
            camera_state->zoom += mouse_wheel * dt;
        }

        //DASH
        if(IsKeyDown(KEY_D) && player.direction != 0 && player.dash_cooldown <= 0) {
            player.dash_cooldown = PLAYER_DASH_COOLDOWN;
            player.dash_time = PLAYER_DASH_LENGTH;
            player.speed_polar.rad = PLAYER_DASH_MAX_SPEED * player.direction;
            player.dashing = true;
        }
            
        // SHOOT
        if(IsKeyDown(KEY_LEFT)) {
            player.direction = 1; 
            player.accelerating = true;
        }
        else if(IsKeyDown(KEY_RIGHT)) {
            player.direction = -1; 
            player.accelerating = true;
        }
        else player.accelerating = false;
        
        if (IsKeyPressed(KEY_SPACE))
            bullets[bullet_count++] = player.Shoot(e_MovementKind::OUTER, e_Team::GOOD_GUYS, 1); 
        if (IsKeyPressed(KEY_X))
            bullets[bullet_count++] = player.Shoot(e_MovementKind::CIRCULAR, e_Team::GOOD_GUYS, -1);
        if (IsKeyPressed(KEY_Z))
            bullets[bullet_count++] = player.Shoot(e_MovementKind::CIRCULAR, e_Team::GOOD_GUYS, 1);
    }
    
};

constexpr int balls_around = 12;

int main(void){

    InitWindow(WIN_WIDTH, WIN_HEIGHT, TITLE);
    SetTargetFPS(60);
    
    Game Game(WIN_WIDTH, WIN_HEIGHT, MAX_ENEMIES);
    f32 dt = 1.0f/60.0f;


    // Add Debug Balls on World surface.
    fvec2Polar d_balls_around[balls_around];
    int step = 360 / balls_around;
    for(int i = 0;  i < balls_around; i++)
    {
        d_balls_around[i].length = CENTER_RADIUS;
        d_balls_around[i].rad = f32(step * i);
    }


    while(!WindowShouldClose())
    {
        Game.ProcessInput(dt);
        Game.UpdatePlayer(dt);
        Game.UpdateOthers(dt);
        Game.Collisions(dt);
        Game.UpdateCamera(dt);

        BeginDrawing();
            BeginMode2D(Game.camera_state);
            Game.Draw();
            for (auto ball: d_balls_around){
                fvec2 p = PolarToCartesian(ball.length, ball.rad * DEG2RAD);
                DrawCircle((i32)p.x, (i32)p.y, 4, YELLOW);
            }
            EndMode2D();
        EndDrawing();
    }
    return 0;
}


