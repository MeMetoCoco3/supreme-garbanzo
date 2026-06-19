#include "game.h"
#include "entity.h"
#include "vstd/vmath.h"
#include "vstd/vrandom_gen.h"
#include <cstddef>


constexpr f32 PLAYER_MAX_SPEED = 15.0f;
constexpr f32 PLAYER_DASH_MAX_SPEED = 25.0f;
constexpr f32 PLAYER_DASH_LENGTH = 0.01f;
constexpr f32 PLAYER_DASH_COOLDOWN = 1.0f;
constexpr f32 PLAYER_ACCELERATION = 30.0f;

Game::Game(i32 w_width, i32 w_height): 
    win_size(w_width, w_height) 
{
    player.movement_kind = e_MovementKind::CIRCULAR;
    player.radius = E_RADIUS;
    player.color = E_COLOR;
    player.polar.rad = PI * 0.5f;
    player.polar.length = CENTER_RADIUS + E_RADIUS;

    camera_state->target = {0, 0};
    camera_state->offset.x = w_width * 0.5f;
    camera_state->offset.y = w_height * 0.5f;
    camera_state->zoom = 0.6f;
}

void Game::NewEnemyCloud(size_t num_enemies, f32 distance_from_surface, i32 direction) {
    for(int i = 0; i < enemy_clouds.size(); i++){
        if (!enemy_clouds[i].is_alive) {
            enemy_clouds[i] = EnemyCloud(num_enemies, distance_from_surface, direction, i);
            return;
        }
    }
    enemy_clouds.emplace_back(EnemyCloud(num_enemies, distance_from_surface, direction, enemy_clouds.size()));
}

void Game::UpdatePlayer(f32 dt) {
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

void Game::UpdateOthers(f32 dt) {
    for (auto& bullet: bullets) {
        bullet.UpdateBullet(dt);
    }
    for (auto& cloud: enemy_clouds) {
        cloud.UpdateEnemies(dt);
    }
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

void Game::UpdateCamera(f32 dt) {
    fvec2Polar player_on_polar = player.polar;
    player_on_polar.length += 50.0f;

    fvec2 player_pos =  PolarToCartesian(player_on_polar.length, player_on_polar.rad);
    camera_state->target.x = player_pos.x;
    camera_state->target.y = player_pos.y;
    camera_state->rotation = (player_on_polar.rad * RAD2DEG) - 90.0f;
}

void Game::Collisions(f32 dt){
    for (Bullet& bullet: bullets){
        if (!bullet.is_alive) continue;
        fvec2 bullet_pos = PolarToCartesian(bullet.polar.length, bullet.polar.rad);

        if (bullet.team == e_Team::GOOD_GUYS) {
            for (EnemyCloud& cloud: enemy_clouds){
                for (EntityFromCloud& enemy: cloud.enemies) {
                    if (!enemy.is_alive) continue;
                    fvec2 enemy_pos = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                    if(CheckCollisionCircles(
                        {bullet_pos.x, bullet_pos.y}, bullet.radius, 
                        {enemy_pos.x, enemy_pos.y}, enemy.radius
                    )){
                        bullet.is_alive = false;
                        enemy.is_alive = false;
                        enemy_clouds[enemy.cloud_idx].count-- ;
                        printf("KILLED ENEMY, NEW COUT: %zd\n", enemy_clouds[enemy.cloud_idx].count);
                        break;
                    }
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

void Game::Draw() {
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


    for (const auto& cloud: enemy_clouds){
        for (const auto& enemy: cloud.enemies){
            if(enemy.is_alive){
                fvec2 enemy_pos = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                DrawCircle((i32)enemy_pos.x, (i32)enemy_pos.y, enemy.radius, enemy.color);
            }
        }
    }
}

void Game::ProcessInput(f32 dt) {
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

constexpr f32 TIME_BETWEEN_CLOUDS = 4.0f;
constexpr size_t MIN_ENEMIES_PER_CLOUD = 8;
constexpr f32 MIN_SPAWN_DISTANCE = 100;
constexpr f32 MAX_SPAWN_DISTANCE = 240;

i32 GetRandomDirection() {
    return (rng::randi32(0, 1) == 0) ? -1 : 1; 
}

SpawSystem::SpawSystem(Game& game): game(game){}

void SpawSystem::Update(f32 dt){
    if (till_next_cloud <= 0.0) {
        till_next_cloud = TIME_BETWEEN_CLOUDS;
        size_t num_enemies = (size_t)rng::randi32((i32)MIN_ENEMIES_PER_CLOUD, (i32) MAX_ENEMIES_PER_CLOUD);
        f32 distance = rng::randf32(MIN_SPAWN_DISTANCE, MAX_SPAWN_DISTANCE);
        i32 direction = GetRandomDirection();

        game.NewEnemyCloud(num_enemies, distance + CENTER_RADIUS, direction);

        printf("WE SPAWN: %zd, %02f, %d \n", num_enemies, distance + CENTER_RADIUS, direction);
    } else {
        till_next_cloud -= dt;
    }
}

