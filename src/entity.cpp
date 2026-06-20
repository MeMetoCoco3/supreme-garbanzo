#include "entity.h"
#include "raylib.h"
#include "vstd/vrandom_gen.h"

#include <array>
#include "vstd/vmath.h"
#include "vstd/vtypes.h"
#include <ctime>


f32 GetNextRadians(f32 radius_main, f32 radius1, f32 radius2) {
    f32 length_sector = radius2+radius1;
    f32 radians = length_sector/radius_main;
    return radians;
}

fvec2 PolarToCartesian(f32 length, f32 rad) {
    return {length * cosf(rad), -(length * sinf(rad))};
}

Entity::Entity(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir):
        radius(radius), color(c), direction(dir), movement_kind(movement), kind(kind)
{
    polar.length = polar_length;
    polar.rad = radians;
    is_alive = true;
}

Bullet::Bullet(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir, e_Team team):
    Entity(radius, c, radians, polar_length, kind, movement, dir), team(team) {}


constexpr f32 Entity::Radius_Speed() {
    return 50.0f;
}

constexpr f32 Entity::Max_Speed() {
    return 100000.0f / polar.length;
}
constexpr f32 Entity::Acceleration() {
    return 500.0f;
}

Bullet Entity::Shoot(e_MovementKind kind, f32 speed_rad, f32 speed_length, e_Team team, i32 dir) {
    Bullet bullet = Bullet(BULLET_RADIUS, BULLET_COLOR, polar.rad, polar.length, e_EntityKind::BULLET, kind, dir, team);
    bullet.speed_polar.length = speed_length;
    bullet.speed_polar.rad = speed_rad;
    return bullet;
}

void Bullet::UpdateBullet(f32 dt){
    switch (movement_kind) {
        case e_MovementKind::NIL: {
            // V_LOG_WARN("MOVEMENT KIND IS NUL FOR BULLET ON LENGTH %02f, at radian %02f\n", this->polar.length, this->polar.rad);
        } break;
        case e_MovementKind::CIRCULAR:{
            polar.rad += speed_polar.rad * dt * direction;      
        } break;
        case e_MovementKind::OUTER:{
            polar.length += speed_polar.length * dt * direction;
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
    f32 radians_step = GetNextRadians(distance_from_surface, ENEMY_RADIUS, ENEMY_RADIUS);
    i32 row_count = 0;
    i32 entities_in_row = 0;
    
    EntityFromCloud e1(ENEMY_RADIUS, PINK, radians_step + starting_point_radians, distance_from_surface, e_EntityKind::ENEMY, e_MovementKind::CIRCULAR, dir, idx);
    enemies[0] = e1;

    for(int i = 0; i < enemy_capacity; i++) {

        if (entities_in_row >= enemies_per_row[row_count]){
            row_count += 1;
            distance_from_surface += 30;   
            entities_in_row = 0;
            radians_step = GetNextRadians(distance_from_surface, ENEMY_RADIUS, ENEMY_RADIUS);

            i32 dif_between_rows = enemies_per_row[row_count] - enemies_per_row[row_count - 1];
            starting_point_radians -= ((f32)dif_between_rows * radians_step) * 0.5f;
        }

        entities_in_row +=1;
        f32 radian_pos = starting_point_radians + radians_step * entities_in_row;

        EntityFromCloud current_e (
            ENEMY_RADIUS, PINK, radian_pos,
            distance_from_surface, e_EntityKind::ENEMY, 
            e_MovementKind::CIRCULAR, dir, idx
        );
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
                bullets[bullet_count++] = 
                    enemies[i].Shoot(
                            e_MovementKind::OUTER, ENEMY_BULLET_SPEED_RAD, 
                            ENEMY_BULLET_SPEED_LENGTH, e_Team::BAD_GUYS, -1);
                    printf("%s",PolarToCartesian(bullets[bullet_count-1].polar.length, bullets[bullet_count-1].polar.rad).to_string().c_str());

                if (bullets[bullet_count-1].polar.length == 0.0f){
                    int j = 2;
                    printf("%d\n", j);
                }
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
            }

            lerp_factor += dt;
             
            if (lerp_factor >= 1.0f)
            {
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
                        const Entity& e = enemies[i];
                        new_pos[i] = e.polar.length - APPROACH_STEP;
                    }
                } else {
                    time_till_approach -= 1;
                    action = MOVING;
                    for(int i = 0; i < enemies.size(); i++)
                    {
                        const Entity& e = enemies[i];
                        new_pos[i] = (direction * E_ANGLE_STEP) + e.polar.rad;
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

EntityFromCloud::EntityFromCloud(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir, size_t cloud_idx):
    Entity(radius, c, radians, polar_length, kind, movement, dir), cloud_idx(cloud_idx)
{}
