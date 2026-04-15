#include <cstdio>
#include <raylib.h>
#include <vector>
#include "vstd/vtypes.h"
#include <math.h>

constexpr i32 WIN_WIDTH = 800;
constexpr i32 WIN_HEIGHT = 800;
constexpr f32 CENTER_RADIUS = 100.0f;
constexpr auto TITLE = "ROTATOS\n";

constexpr f32 E_RADIUS = 20.0f;
constexpr Color E_COLOR = YELLOW;
constexpr f32 BULLET_RADIUS = E_RADIUS * 0.5f;
constexpr Color BULLET_COLOR = BLUE;
constexpr f32 BULLET_SPEED = 40.0f;
constexpr f32 ENEMY_POLAR_LENGTH = 300.0f;
constexpr f32 ENEMY_RADIUS = 10.0f;

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
    
    void UpdateBullet(f32 dt)
    {
        polar.length += BULLET_SPEED * dt;
        position = PolarToCartesian(polar.length, polar.rad);
    }

    void Update(f32 dt){
        speed += Acceleration() * (f32)direction * dt;
        f32 delta = speed / polar.length;
        
        polar.rad += delta * dt;
        
        position = PolarToCartesian(polar.length, polar.rad);
    }

    const f32 Radius_Speed(){
        return 50.0f;
    }

    const f32 Max_Speed(){
        return 40.0f / polar.length;
    }
    const f32 Acceleration(){
        return 150.0f;
    }
};




void DrawBg(){
    DrawCircle(0, 0, CENTER_RADIUS, RED);

    DrawCircleLines(0, 0, CENTER_RADIUS, BLACK);
}

constexpr i32 ENEMY_COUNT = 28;
namespace Row{
    constexpr i32 ROW_0 = 10,
                ROW_1 = 12,
                ROW_2 = 6;
    constexpr i32 rows_length[3] {ROW_0, ROW_1, ROW_2};
}

// WARN: It is an aproximation
f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2) {
    f32 length_sector = radius2+radius1;
    f32 radians = length_sector/radius_main;
    printf("%02f\n", radians);
    return radians;
}

int main(void){
    printf("hello world\n");
    
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
    bullets.reserve(40);

    std::vector<Entity>enemies;
    enemies.reserve(40);
    
    
    {   

        f32 row_polar_length = CENTER_RADIUS * 2.0f;

        Entity e1(ENEMY_RADIUS, PINK, 0.0f, row_polar_length, Entity::ENEMY);
        e1.position = PolarToCartesian(e1.polar.length, e1.polar.rad);
        e1.direction = 1;
        enemies.push_back(e1);
        
        i32 row_count = 0;
        i32 entities_in_row = 0;
        for(int i = 1; i < ENEMY_COUNT; i++) {
            if (entities_in_row >= Row::rows_length[row_count]){
                row_count += 1;
                row_polar_length += 30;   
                entities_in_row = 0;
            }

            entities_in_row +=1;

            f32 radians = GetNextRadians(row_polar_length, ENEMY_RADIUS, ENEMY_RADIUS);
            Entity current_e(ENEMY_RADIUS, PINK, radians * entities_in_row, row_polar_length, Entity::ENEMY);
            current_e.position = PolarToCartesian(current_e.polar.length, current_e.polar.rad-enemies[i-1].polar.rad);
            current_e.direction = 1;
            printf("E %d R %d position %s\n", entities_in_row, row_count, current_e.position.to_string().c_str());
            enemies.push_back(current_e);
        }
    }
    while(!WindowShouldClose())
    {
        f32 mouse_wheel = GetMouseWheelMove();   
        if(mouse_wheel != 0){
            c.zoom += mouse_wheel * dt;
        }
        
        e.direction = 0;
        if(IsKeyDown(KEY_DOWN))
            e.direction = 1;

        if(IsKeyDown(KEY_UP))
            e.direction = -1;

        if(IsKeyDown(KEY_Z))
            e.polar.length -= e.Radius_Speed() * dt;

        if(IsKeyDown(KEY_X))
            e.polar.length += e.Radius_Speed() * dt;
        if (IsKeyPressed(KEY_SPACE))
            e.Shoot(bullets);

        e.Update(dt);
        
        for (auto& bullet: bullets){
            bullet.UpdateBullet(dt);
        }
        for (auto& enemy: enemies){
            enemy.Update(dt);
        }
        BeginDrawing();
            BeginMode2D(c);
            ClearBackground(RAYWHITE);
            DrawBg();
            DrawCircle((i32)e.position.x, (i32)e.position.y, e.radius, e.color);
            for (const auto& bullet: bullets){
                DrawCircle((i32)bullet.position.x, (i32)bullet.position.y, bullet.radius, bullet.color);
            }
            for (const auto& enemy: enemies){
                DrawCircle((i32)enemy.position.x, (i32)enemy.position.y, enemy.radius, enemy.color);
            }
            EndMode2D();
        EndDrawing();
    }
    return 0;
}
