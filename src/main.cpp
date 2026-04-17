#include <cmath>
#include <cstddef>
#include <cstdio>
#include <raylib.h>
#include <vector>
#include "vstd/vmath.h"
#include "vstd/vtypes.h"
#include <math.h>

constexpr i32 WIN_WIDTH = 800;
constexpr i32 WIN_HEIGHT = 800;
constexpr f32 CENTER_RADIUS = 100.0f;
constexpr auto TITLE = "ROTATOS\n";

constexpr f32 FLOOR_FRICTION = 0.80f;
constexpr f32 E_RADIUS = 20.0f;
constexpr Color E_COLOR = YELLOW;
constexpr f32 BULLET_RADIUS = E_RADIUS * 0.8f;
constexpr Color BULLET_COLOR = BLUE;
constexpr f32 BULLET_SPEED = 220.0f;
constexpr f32 ENEMY_RADIUS = 10.0f;

namespace Row{
    constexpr i32 ROW_0 = 10,
                ROW_1 = 12,
                ROW_2 = 6;
    constexpr i32 rows_length[3] {ROW_0, ROW_1, ROW_2};
}

f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2);
fvec2 PolarToCartesian(f32 length, f32 rad){
    return {length * cosf(rad), length * sinf(rad)};
}



struct Entity {
    fvec2 position = {0,0};
    struct {
        f32 length = 0.0f;
        f32 rad = 0.0f;
    } polar;
    f32 radius = 0;
    f32 speed = 0;
    i32 direction = 0;
    Color color = BLANK;
    bool is_alive = true;
    enum e_Kind { 
        PLAYER, 
        BULLET,
        ENEMY
    } kind;

    Entity(f32 radius, Color c, f32 radians, f32 polar_length, e_Kind kind):
        radius(radius), color(c), kind(kind)
    {
        polar.length = polar_length;
        polar.rad = radians;
    }

    
    void Shoot(std::vector<Entity>& bullets) {
        bullets.push_back(Entity(BULLET_RADIUS, BULLET_COLOR, polar.rad, polar.length, BULLET));
    }

    void UpdateBullet(f32 dt) {
        polar.length += BULLET_SPEED * dt;
        position = PolarToCartesian(polar.length, polar.rad);
    }
    
    void Update(f32 dt) {
        if (is_alive) {
            speed += Acceleration() * (f32)direction * dt;
            if(direction == 0 || direction != Signf32((f32)speed)) speed *= FLOOR_FRICTION;
            speed = Clampf32(speed, -Max_Speed(), Max_Speed());
            f32 delta = speed / polar.length;
            
            polar.rad += delta * dt;
            position = PolarToCartesian(polar.length, polar.rad);


            polar.rad = fmodf(polar.rad, 2.0f * PI);
            if (polar.rad < 0) polar.rad += 2.0f * PI;
        }
    }

    constexpr f32 Radius_Speed() {
        return 50.0f;
    }

    constexpr f32 Max_Speed(){
        return 100000.0f / polar.length;
    }
    constexpr f32 Acceleration(){
        return 500.0f;
    }
};

struct EnemySystem {
    std::vector<Entity> enemies;
    std::vector<f32> new_angle;
    f32 delay = 0.0f;
    f32 lerp_factor = 0.0f;
    enum {
        WAITING, 
        MOVING
    } action = WAITING;

    const f32 E_ANGLE_STEP = 1.0f;
    const f32 E_TIME_STEP = 1.0f;
    EnemySystem(size_t enemy_capacity){
        enemies.reserve(enemy_capacity);
        new_angle.resize(enemy_capacity);

        f32 row_polar_length = CENTER_RADIUS * 2.0f;

        Entity e1(ENEMY_RADIUS, PINK, 0.0f, row_polar_length, Entity::ENEMY);
        e1.position = PolarToCartesian(e1.polar.length, e1.polar.rad);
        e1.direction = 1;
        enemies.push_back(e1);

        i32 row_count = 0;
        i32 entities_in_row = 0;
        for(int i = 1; i < enemy_capacity; i++) {
            if (entities_in_row >= Row::rows_length[row_count]){
                row_count += 1;
                row_polar_length += 30;   
                entities_in_row = 0;
                printf("new row%d with capcaity for %d\n", row_count, Row::rows_length[row_count]);
                if (row_count == 3) break;
            }

            printf("i = %d n = %d\n", i, entities_in_row);

            f32 radians = GetNextRadians(row_polar_length, ENEMY_RADIUS, ENEMY_RADIUS);

            Entity current_e(ENEMY_RADIUS, PINK, radians * entities_in_row, row_polar_length, Entity::ENEMY);
            current_e.position = PolarToCartesian(current_e.polar.length, current_e.polar.rad);
            current_e.direction = 1;
            enemies.push_back(current_e);
            entities_in_row +=1;
            
        }


    }

    void UpdateEnemies(f32 dt) {

        switch (action)
        {
            case WAITING: {
                if(delay >= E_TIME_STEP) {
                    delay = 0.0f;
                    action = MOVING;
                    f32 previous_polar_length = 0.0f;
                    f32 previous_angle_step = 0.0f;
                    for(int i = 0; i < enemies.size(); i++)
                    {
                        const Entity& e = enemies[i];
                        if (previous_polar_length != e.polar.length) {
                            previous_polar_length = e.polar.length;
                            previous_angle_step = E_ANGLE_STEP;
                        }
                        
                        new_angle[i] = previous_angle_step + e.polar.rad;
                        
                    }
                } else {
                    delay += dt;
                }
            } break;
            case MOVING: {
                for(int i = 0; i < enemies.size(); i++)
                {
                    f32 delta = new_angle[i] - enemies[i].polar.rad;
                    while (delta > PI)  delta -= 2.0f * PI;
                    while (delta < -PI) delta += 2.0f * PI;

                    f32 target = enemies[i].polar.rad + delta;
                    enemies[i].polar.rad = Lerp(enemies[i].polar.rad, target, lerp_factor);
                    enemies[i].position = PolarToCartesian(enemies[i].polar.length, enemies[i].polar.rad);

    
                }
                lerp_factor += dt;
                if (lerp_factor >= 1.0f)
                {
                    lerp_factor = 0.0f;
                    delay = 0.0f;
                    action = WAITING;
                }
            } break;
        }
    }
};


void DrawBg(){
    DrawCircle(0, 0, CENTER_RADIUS, RED);
    DrawCircleLines(0, 0, CENTER_RADIUS, BLACK);
}



// WARN: It is an aproximation
f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2) {
    f32 length_sector = radius2+radius1;
    f32 radians = length_sector/radius_main;
    return radians;
}

void CheckCollisions(std::vector<Entity>& bullets, std::vector<Entity>& enemies)
{
    for (Entity& bullet: bullets){
        if (!bullet.is_alive) continue;
        for (Entity& enemy: enemies) {
            if (!enemy.is_alive) continue;
            if(CheckCollisionCircles(
                {bullet.position.x, bullet.position.y}, bullet.radius, 
                {enemy.position.x, enemy.position.y}, enemy.radius
            )){
                bullet.is_alive = false;
                enemy.is_alive = false;
                break;
            }
        }
    }
}




int main(void){
    InitWindow(WIN_WIDTH, WIN_HEIGHT, TITLE);
    
    Camera2D c = {};
    c.target = {0, 0};
    c.offset.x = WIN_WIDTH * 0.5f;
    c.offset.y = WIN_HEIGHT * 0.5f;
    c.zoom = 1.0f;

    Entity e(E_RADIUS, E_COLOR, 0.0f, CENTER_RADIUS - E_RADIUS, Entity::PLAYER);


    SetTargetFPS(60);
    f32 dt = 1.0f/60.0f;

    std::vector<Entity>bullets;
    bullets.reserve(400);

    EnemySystem enemy_sys(40);
   


    while(!WindowShouldClose())
    {
        f32 mouse_wheel = GetMouseWheelMove();   
        if(mouse_wheel != 0){
            c.zoom += mouse_wheel * dt;
        }
        
        if(IsKeyDown(KEY_RIGHT)) e.direction = 1;
        else if(IsKeyDown(KEY_LEFT)) e.direction = -1;
        else e.direction = 0;

        if(IsKeyDown(KEY_L))
            e.polar.length -= e.Radius_Speed() * dt;

        if(IsKeyDown(KEY_X))
            e.polar.length += e.Radius_Speed() * dt;
        if (IsKeyPressed(KEY_SPACE))
            e.Shoot(bullets);

        e.Update(dt);
        
        for (auto& bullet: bullets) {
            bullet.UpdateBullet(dt);
        }
            
        enemy_sys.UpdateEnemies(dt);
        CheckCollisions(bullets, enemy_sys.enemies);
        BeginDrawing();
            BeginMode2D(c);
            ClearBackground(RAYWHITE);
            DrawBg();
            DrawCircle((i32)e.position.x, (i32)e.position.y, e.radius, e.color);
            for (const auto& bullet: bullets){
                if(bullet.is_alive)
                    DrawCircle((i32)bullet.position.x, (i32)bullet.position.y, bullet.radius, bullet.color);
            }
            for (const auto& enemy: enemy_sys.enemies){
                if(enemy.is_alive)
                    DrawCircle((i32)enemy.position.x, (i32)enemy.position.y, enemy.radius, enemy.color);
            }
            EndMode2D();
        EndDrawing();
    }
    return 0;
}
