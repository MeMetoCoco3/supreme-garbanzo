#pragma once 
#include "vstd/vtypes.h"
#include "raylib.h"
#include <array>
#include <cstdio>
#include <memory>
#include <functional>

constexpr i32 NUM_BULLETS = 1024;
constexpr Color E_COLOR = YELLOW;
constexpr f32 CENTER_RADIUS = 240.0f;
constexpr f32 FLOOR_FRICTION = 0.85f;

constexpr f32 E_RADIUS = 20.0f;
constexpr f32 BULLET_RADIUS = E_RADIUS * 0.8f;
constexpr Color BULLET_COLOR = BLUE;
constexpr f32 BULLET_SPEED_CIRCULAR = 0.4f;
constexpr f32 BULLET_SPEED_OUTER = 180.0f;
constexpr f32 TIME_TO_DIE = 12.0f;

constexpr f32 ENEMY_RADIUS = 10.0f;
constexpr i32 MAX_ENEMIES = 100;



f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2);
fvec2 PolarToCartesian(f32 length, f32 rad);

enum class e_Team {
    NIL,
    GOOD_GUYS,
    BAD_GUYS
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

enum class e_BulletKind {
    NIL,
    FROM_CLOUD,
    FROM_WAVER, 
    FROM_BULLER,
    COUNT,
};

enum class e_UpgradeKind {
    NIL = -1,
    STRENGTH = 0,
    SPEED, 
    GO_THROUGH,
    WAVY_SHOOT,
    BUILDING_POINT,
    COUNT,
};


enum class e_BuildingKind {
    NIL,
    V_SHOT,         // VERTICAL SHOT
    THORN_FIELD,    // TAKES HEALTH FROM WHOEVER STEPS ON IT
    DRILL_STATION,  // ALLOWS YOU TO GO TO DRILL STATION ON OPOSITE SIDE.
    COUNT
};

struct Entity {
    fvec2Polar polar;
    fvec2Polar speed_polar = {0, 0};

    i32 health = 0;
    f32 size = E_RADIUS * 10.0f;
    f32 max_speed = 0.0f;
    i32 direction = 0;
    i32 power_level = 0;

    Color color;
    bool is_alive = false;
    bool accelerating = false;
    bool wavy_shoots = false;

    e_BulletKind bullet_kind;
    e_MovementKind movement_kind = e_MovementKind::NIL;

    // WAVY_SHOOTS
    f32 t = 0.0f;
    f32 t_sin = 0.0f;
    f32 prev_t_sin = 0.0f;
    
    Entity() = default;
    Entity(f32 size, Color c, f32 radians, f32 polar_length,  e_MovementKind movement, i32 dir);
    void FixRad();
    Bullet Shoot();
};

enum class e_PlayerState{
    SHOOTING,
    BUILDING
};


struct Player: public Entity {
    e_PlayerState state = e_PlayerState::SHOOTING;
    fvec2Polar max_bullet_speed;

    i32 score = 0;
    i32 building_points = 0;
    e_BuildingKind carrying = e_BuildingKind::NIL;
    // DASHING
    bool dashing = false;
    f32 dash_time = 0.0f;
    f32 dash_cooldown = 0.0f;
    
    f32 size_bullet = 20.0f;

    Player() = default;
    Player(f32 size, Color c, f32 radians, f32 polar_length,  e_MovementKind movement, i32 dir);
    Bullet Shoot();
    Bullet ShootH(i32 dir);
    Bullet ShootV();
    bool IsCarrying();
};

constexpr i32 HEALTH_ENTITY_FROM_CLOUD = 3;

struct EntityFromCloud: public Entity {
    i32 health = HEALTH_ENTITY_FROM_CLOUD; 
    f32 size = E_RADIUS;
    size_t cloud_idx;

    EntityFromCloud() = default;
    EntityFromCloud(Color c, f32 radians, f32 polar_length, e_MovementKind movement, i32 dir, size_t cloud_idx);
};

struct Bullet: public Entity {
    e_Team team = e_Team::NIL;
    f32 time_to_die = 0.0f;
    i32 damage = 1;
    bool wavy = false;
    Bullet() = default;
    Bullet(fvec2Polar);
    void Update(f32 dt);  
};


constexpr size_t MAX_ENEMIES_PER_CLOUD = 12;
constexpr i32 ENEMY_CLOUD_DEFAULT_APPROACH_TIMER = 5;

struct EnemyCloud {
    std::array<EntityFromCloud, MAX_ENEMIES_PER_CLOUD> enemies{};
    std::array<f32, MAX_ENEMIES_PER_CLOUD> new_pos{};
    
    size_t initial_count = 0;
    size_t count = 0;
    bool is_alive = false;

    // Movement
    f32 delay = 0.0f;
    f32 lerp_factor = 0.0f;
    i32 direction;
    i32 time_till_approach = ENEMY_CLOUD_DEFAULT_APPROACH_TIMER;

    // Shooting
    f32 time_till_shoot = 1.0f;
    
    enum {
        WAITING, 
        MOVING,
        APPROACH,
    } action = WAITING;
    
    // If this was const, it could NOT be copied, because that would delete it.
    static constexpr f32 E_ANGLE_STEP = 0.5f;
    static constexpr f32 E_TIME_STEP = 1.0f;

    EnemyCloud(size_t enemy_capacity, f32 distance_from_surface, i32 direction, size_t idx);
    void UpdateEnemies(f32 dt);
    void Shoot(std::array<Bullet, NUM_BULLETS>& bullets, size_t& bullet_count, f32 dt);
};

constexpr size_t MAX_NUM_EXP_PARTICLES = 1024;
constexpr size_t MAX_UPGRADES = 6;

struct Upgrade {
    std::string name = "";
    e_UpgradeKind kind = e_UpgradeKind::NIL;
    i32 level = 1;
    std::function<void(Player&)> command;
    Texture2D image;
    Upgrade() = default;
    Upgrade(e_UpgradeKind kind, std::string name, std::function<void(Player&)> func, Texture2D image);
};

struct Building {
    static constexpr f32 size = 0;

    fvec2Polar polar = {0};
    e_BuildingKind kind = e_BuildingKind::NIL;   
    i32 index = 0;
    i32 brother_idx = -1; // No problem cause there is no swaps on our ds.
    i32 lvl = 0;


    Building() = default;
    Building(fvec2Polar, e_BuildingKind, i32, i32);
};


namespace pbs {
    namespace buildings{
        static std::unique_ptr<std::array<Building, (i32)e_BuildingKind::COUNT>> __load();
        Building Get(e_BuildingKind);
    };

    namespace bullets {
        static std::unique_ptr<std::array<Bullet, (i32)e_BulletKind::COUNT>> __load();
        Bullet Get(e_BulletKind);
    };

    namespace upgrades {
        static std::unique_ptr<std::array<Upgrade, (i32) e_UpgradeKind::COUNT>> __load();
        Upgrade Get(e_UpgradeKind);
    };
};

