#include "entity.h"
#include "raylib.h"
#include "vstd/vrandom_gen.h"
#include "resources.h"

#include "vstd/vlogger.h"
#include <array>
#include "vstd/vmath.h"
#include "vstd/vtypes.h"
#include <cstddef>
#include <ctime>


f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2) {
    f32 length_sector = radius2+radius1;
    f32 radians = length_sector/radius_main;
    return radians;
}

fvec2 PolarToCartesian(f32 length, f32 rad) {
    return {length * cosf(rad), -(length * sinf(rad))};
}


constexpr i32 MIN_POWER_LEVEL = 2;
constexpr i32 MAX_POWER_LEVEL = 5;

Entity::Entity(f32 radius, Color c, f32 radians, f32 polar_length,  e_MovementKind movement, i32 dir):
       size(radius), direction(dir), color(c), movement_kind(movement)
{
    polar.length = polar_length;
    polar.rad = radians;
    is_alive = true;
    power_level = rng::randi32(MIN_POWER_LEVEL, MAX_POWER_LEVEL);
}


Bullet Entity::Shoot() {
    Bullet bullet = pbs::bullets::Get(bullet_kind);
    bullet.polar = polar;
    bullet.is_alive = true;
    return bullet;
}

void Entity::FixRad(){
    polar.rad = fmodf(polar.rad, PI * 2.0f);
    if (polar.rad < 0) polar.rad += PI * 2.0f;
}

constexpr f32 WAVY_STRENGTH_LENGTH = 40.0f;
constexpr f32 WAVY_STRENGTH_RAD = 0.02f;
constexpr f32 WAVY_OSCILATION_SPEED = 10.0f;
void Bullet::Update(f32 dt){
    t += dt * WAVY_OSCILATION_SPEED;
    prev_t_sin = t_sin;
    t_sin = sin(t);
    switch (movement_kind) {
        case e_MovementKind::NIL: {
            V_LOG_WARN("MOVEMENT KIND IS NUL FOR BULLET ON LENGTH %02f, at radian %02f\n", this->polar.length, this->polar.rad);
        } break;
        case e_MovementKind::CIRCULAR:{
            polar.rad += speed_polar.rad * dt * direction;      
            printf("CIRCULAR: %d\n", direction); 
            if(wavy) {
                t_sin *= WAVY_STRENGTH_LENGTH;
                polar.length += t_sin - prev_t_sin;
            }
        } break;
        case e_MovementKind::OUTER:{
            polar.length += speed_polar.length * dt * direction;
            if(wavy) {
                t_sin *= WAVY_STRENGTH_RAD;
                polar.rad += t_sin - prev_t_sin;
            }
        } break;
    }
    time_to_die += dt;
    if (time_to_die >= TIME_TO_DIE) is_alive = false;
}

EnemyCloud::EnemyCloud(size_t enemy_capacity, f32 distance_from_surface, i32 dir, size_t idx):
    direction(dir) 
{
    if (enemy_capacity > MAX_ENEMIES_PER_CLOUD) {
        enemy_capacity = MAX_ENEMIES_PER_CLOUD;
    }

    count = enemy_capacity;
    initial_count = enemy_capacity;

    i32 enemies_per_row[3] = {0};
    {
        i32 enemy_counter = (i32) enemy_capacity;
        i32 next_row = rng::randi32(0, 2);

        for (int i = 0; i < 3; i++) {
            i32 enemies_in_next_row;
            if(i < 2) {
                enemies_in_next_row = i32(enemy_counter / 2);
            } else {
                enemies_in_next_row = enemy_counter;
            }
            enemies_per_row[next_row] = enemies_in_next_row;
            enemy_counter -= enemies_in_next_row;

            next_row = (next_row + 1) % 3;
        }
    }
    
    f32 starting_point_radians = rng::randf32(0.0f, 2.0f * PI);
    i32 row_count = 0;
    i32 entities_in_row = 0;
    
    f32 e_size = EntityFromCloud().size;
    f32 radians_step = GetNextRadians(distance_from_surface, e_size, e_size);

    for(int i = 0; i < enemy_capacity; i++) {
        f32 radian_pos = starting_point_radians + radians_step * entities_in_row;
        EntityFromCloud current_e (
            PINK, radian_pos, distance_from_surface,  
            e_MovementKind::CIRCULAR, dir, idx
        );

        if (entities_in_row >= enemies_per_row[row_count]){
            row_count += 1;
            distance_from_surface += 30;   
            entities_in_row = 0;
            radians_step = GetNextRadians(distance_from_surface, e_size, e_size);

            i32 dif_between_rows = enemies_per_row[row_count] - enemies_per_row[row_count - 1];
            starting_point_radians -= ((f32)dif_between_rows * radians_step) * 0.5f;
        }

        entities_in_row +=1;

        Color c = i == 0? RED: PINK;
        current_e.color = c;
        current_e.bullet_kind = e_BulletKind::FROM_CLOUD;
        enemies[i] = current_e;
    }

    is_alive = true;
}


constexpr f32 ENEMY_BULLET_SPEED_LENGTH = 150.0f;
constexpr f32 ENEMY_BULLET_SPEED_RAD = 0.3f;

void EnemyCloud::Shoot(std::array<Bullet, NUM_BULLETS>& bullets, size_t& bullet_count, f32 dt)
{
    if (time_till_shoot <= 0) {
        i32 random_entity = rng::randi32(0, (i32)initial_count-1);
        for (int i = random_entity; i <= (i32)initial_count; i++) {
            if (enemies[i].is_alive) {
                Bullet bullet = enemies[i].Shoot();
                bullet.speed_polar.length = ENEMY_BULLET_SPEED_LENGTH;
                bullet.direction = -1;
                bullets[bullet_count++] = bullet;
                
                break;
            }
        }
        time_till_shoot = rng::randf32(0.4f, 2.0f);
    } else {
        time_till_shoot -= dt;
    }
}

constexpr f32 APPROACH_STEP = 50;

void EnemyCloud::UpdateEnemies(f32 dt) {
    switch (action)
    {
        case APPROACH: {
            for(int i = 0; i < enemies.size(); i++)
            {
                Entity& e = enemies[i];
                e.polar.length = fLerp(e.polar.length, new_pos[i], lerp_factor);
                e.FixRad();
            }

            lerp_factor += dt;
             
            if (lerp_factor >= 1.0f)
            {
                direction = -direction;
                lerp_factor = 0.0f;
                delay = 0.0f;
                action = WAITING;
            }
            
        } break;
        case WAITING: {
            if(delay >= E_TIME_STEP) {
                delay = 0.0f;
                if (time_till_approach == 0) {
                    time_till_approach = ENEMY_CLOUD_DEFAULT_APPROACH_TIMER;
                    action = APPROACH;
                    for(int i = 0; i < enemies.size(); i++)
                    {
                        Entity& e = enemies[i];
                        new_pos[i] = e.polar.length - APPROACH_STEP;
                        e.FixRad();
                    }
                } else {
                    time_till_approach -= 1;
                    action = MOVING;
                    for(int i = 0; i < enemies.size(); i++)
                    {
                        Entity& e = enemies[i];
                        new_pos[i] = (direction * E_ANGLE_STEP) + e.polar.rad;
                        e.FixRad();
                    }
                }
            } else {
                delay += dt;
            }
        } break;
        case MOVING: {
            for(int i = 0; i < enemies.size(); i++)
            {
                Entity& e = enemies[i];
                e.polar.rad = fLerp(e.polar.rad, new_pos[i], lerp_factor);
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

EntityFromCloud::EntityFromCloud(Color c, f32 radians, f32 polar_length, e_MovementKind movement, i32 dir, size_t cloud_idx):
    Entity(0, c, radians, polar_length, movement, dir), cloud_idx(cloud_idx)
{}

Upgrade::Upgrade(e_UpgradeKind kind, std::string name, std::function<void(Player&)> func, Texture2D image): name(name), kind(kind), command(func), image(image){}

std::unique_ptr<std::array<Upgrade, (i32) e_UpgradeKind::COUNT>> pbs::upgrades::__load(){
    auto upgrades = std::make_unique<std::array<Upgrade, (i32) e_UpgradeKind::COUNT>>();

    upgrades->at((size_t) e_UpgradeKind::STRENGTH) = Upgrade(e_UpgradeKind::STRENGTH, "STRENGTH", [](Player& e) {
       e.power_level += 1; 
    }, Resources::get_texture("upgrade_strength"));

    upgrades->at((size_t) e_UpgradeKind::SPEED) = Upgrade(e_UpgradeKind::SPEED, "SPEED", [](Player& e) {
       e.max_speed += 1; 
    }, Resources::get_texture("upgrade_speed"));

    upgrades->at((size_t) e_UpgradeKind::GO_THROUGH) = Upgrade(e_UpgradeKind::GO_THROUGH, "GO_THROUGH", [](Player& e) {
       e.max_speed += 1; 
    }, Resources::get_texture("upgrade_strength"));

    upgrades->at((size_t) e_UpgradeKind::WAVY_SHOOT) = Upgrade(e_UpgradeKind::WAVY_SHOOT, "WAVY_SHOOT", [](Player& e) {
       e.wavy_shoots = true;
    }, Resources::get_texture("upgrade_wavy"));

    upgrades->at((size_t) e_UpgradeKind::BUILDING_POINT) = Upgrade(e_UpgradeKind::BUILDING_POINT, "BUILDING_POINT", [](Player& e) {
        e.building_points += 1;
    }, Resources::get_texture("upgrade_build"));

    

    return upgrades;
}

Upgrade pbs::upgrades::Get(e_UpgradeKind idx){
    static auto upgrades = __load();
    return upgrades->at((size_t)idx);
}

std::unique_ptr<std::array<Bullet, (i32)e_BulletKind::COUNT>> pbs::bullets::__load(){
    auto bullets = std::make_unique<std::array<Bullet, (i32) e_BulletKind::COUNT>>();
    {
        auto& bullet = bullets->at((size_t) e_BulletKind::FROM_CLOUD);
        bullet.size = 10.0f;
        bullet.color = RED;
        bullet.movement_kind = e_MovementKind::OUTER;
        bullet.team = e_Team::BAD_GUYS;
    }

    {
        auto& bullet = bullets->at((size_t) e_BulletKind::FROM_WAVER);
        bullet.size = 15.0f;
        bullet.color = PURPLE;
        bullet.movement_kind = e_MovementKind::OUTER;
        bullet.team = e_Team::BAD_GUYS;
        bullet.wavy = true;
    }

    {
        auto& bullet = bullets->at((size_t) e_BulletKind::FROM_BULLER);
        bullet.size = 40.0f;
        bullet.color = YELLOW;
        bullet.movement_kind = e_MovementKind::CIRCULAR;
        bullet.team = e_Team::BAD_GUYS;
    }
       
    return bullets;
}

Bullet pbs::bullets::Get(e_BulletKind idx){
    static auto bullets = __load();
    return bullets->at((size_t)idx);
}
// This now does not look necessary, but maybe in the future it does.
std::unique_ptr<std::array<Building, (i32)e_BuildingKind::COUNT>> pbs::buildings::__load(){
    auto buildings = std::make_unique<std::array<Building, (i32) e_BuildingKind::COUNT>>();
    
    {
        auto& building = buildings->at((size_t) e_BuildingKind::V_SHOT);  
        building.kind = e_BuildingKind::V_SHOT;
    }

    {
        auto& building = buildings->at((size_t) e_BuildingKind::THORN_FIELD);  
        building.kind = e_BuildingKind::THORN_FIELD;
    }

    {
        auto& building = buildings->at((size_t) e_BuildingKind::DRILL_STATION);  
        building.kind = e_BuildingKind::DRILL_STATION;
    }

    return buildings;
}

Building pbs::buildings::Get(e_BuildingKind idx){
    static auto buildings = __load();
    return buildings->at((size_t)idx);
}




Bullet Player::Shoot() {
    V_LOG_ERROR("PLAYER SHOULD NOT USE SHOOT.");
    return Bullet();
} 

Bullet Player::ShootH(i32 dir){
    Bullet bul = {};
    bul.polar = polar;
    bul.team = e_Team::GOOD_GUYS;
    bul.wavy = wavy_shoots;
    bul.color = BLUE;
    bul.size = size_bullet;
    bul.is_alive = true;
    bul.movement_kind = e_MovementKind::CIRCULAR;
    bul.power_level = power_level;
    bul.speed_polar.rad = max_bullet_speed.rad;
    bul.direction = dir;
    return bul; 
}

Bullet Player::ShootV(){
    Bullet bul = {};
    bul.polar = polar;
    bul.team = e_Team::GOOD_GUYS;
    bul.wavy = wavy_shoots;
    bul.color = BLUE;
    bul.size = size_bullet;
    bul.is_alive = true;
    bul.movement_kind = e_MovementKind::OUTER;
    bul.power_level = power_level;
    bul.speed_polar.length = max_bullet_speed.length;
    bul.direction = 1;
    return bul; 
}

bool Player::IsCarrying(){
    return carrying != e_BuildingKind::NIL;
}


