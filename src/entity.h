#pragma once 
#include "vstd/vtypes.h"
#include "raylib.h"
#include <array>
#include <cstdio>

constexpr f32 E_RADIUS = 20.0f;
constexpr Color E_COLOR = YELLOW;
constexpr f32 BULLET_RADIUS = E_RADIUS * 0.8f;
constexpr Color BULLET_COLOR = BLUE;
constexpr f32 BULLET_SPEED_CIRCULAR = 3.0f;
constexpr f32 BULLET_SPEED_OUTER = 220.0f;
constexpr i32 NUM_BULLETS = 100;
constexpr f32 TIME_TO_DIE = 1.0f;
constexpr f32 CENTER_RADIUS = 400.0f;
constexpr f32 FLOOR_FRICTION = 0.90f;
constexpr f32 ENEMY_RADIUS = 10.0f;
constexpr i32 MAX_ENEMIES = 100;

namespace Row{
    constexpr i32 ROW_0 = 10,
                ROW_1 = 12,
                ROW_2 = 6;
    constexpr i32 rows_length[3] {ROW_0, ROW_1, ROW_2};
}



f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2);
fvec2 PolarToCartesian(f32 length, f32 rad);

enum class e_Team {
    NIL,
    GOOD_GUYS,
    BAD_GUYS
};

enum class e_EntityKind {
    NIL,
    ENEMY,
    BULLET,
};

enum class e_MovementKind {
    NIL,
    CIRCULAR,
    OUTER
};


struct fvec2Polar {
    f32 length = 0.0f;
    f32 rad = 0.0f;
};

struct Bullet;

struct Entity {
    fvec2Polar polar;
    f32 radius = 0;

    fvec2Polar speed_polar = {0,0};

    Color color;
    bool is_alive = true;
    i32 direction = 0;

    bool accelerating = false;


    e_MovementKind movement_kind = e_MovementKind::NIL;
    e_EntityKind kind = e_EntityKind::NIL;

    // DASHING
    bool dashing = false;
    f32 dash_time = 0.0f;
    f32 dash_cooldown = 0.0f;
    
    Entity() = default;
    Entity(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir);

    Bullet Shoot(e_MovementKind kind, e_Team team, i32 dir);   
    
    constexpr f32 Radius_Speed();
    constexpr f32 Max_Speed();
    constexpr f32 Acceleration();
};

struct Bullet: public Entity {
    e_Team team;
    f32 time_to_die = 0.0f;
    Bullet() = default;
    Bullet(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir, e_Team team);
    void UpdateBullet(f32 dt);  
};


constexpr size_t MAX_ENEMIES_PER_CLOUD = 24;
struct EnemyCloud {
    std::array<Entity, MAX_ENEMIES_PER_CLOUD> enemies{};
    std::array<f32, MAX_ENEMIES_PER_CLOUD> new_angle{};

    // Movement
    f32 delay = 0.0f;
    f32 lerp_factor = 0.0f;

    // Shooting
    f32 time_till_shoot = 1.0f;

    i32 direction;

    enum {
        WAITING, 
        MOVING
    } action = WAITING;

    const f32 E_ANGLE_STEP = 1.0f;
    const f32 E_TIME_STEP = 1.0f;

    EnemyCloud(size_t enemy_capacity, f32 distance_from_surface, i32 direction);
    void UpdateEnemies(f32 dt);
};
