#include "entity.h"
#include "raylib.h"

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
}

Bullet::Bullet(f32 radius, Color c, f32 radians, f32 polar_length, e_EntityKind kind, e_MovementKind movement, i32 dir, e_Team team):
    Entity(radius, c, radians, polar_length, kind, movement, dir), team(team) {}


constexpr f32 Entity::Radius_Speed() {
    return 50.0f;
}

constexpr f32 Entity::Max_Speed(){
    return 100000.0f / polar.length;
}
constexpr f32 Entity::Acceleration(){
    return 500.0f;
}

Bullet Entity::Shoot(e_MovementKind kind, e_Team team, i32 dir) {
    Bullet bullet = Bullet(BULLET_RADIUS, BULLET_COLOR, polar.rad, polar.length, e_EntityKind::BULLET, kind, dir, team);
    bullet.speed_polar.length = BULLET_SPEED_OUTER;
    bullet.speed_polar.rad = BULLET_SPEED_CIRCULAR;
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

EnemyCloud::EnemyCloud(size_t enemy_capacity, f32 distance_from_surface, i32 dir){
    if (enemy_capacity > MAX_ENEMIES_PER_CLOUD) {
        enemy_capacity = MAX_ENEMIES_PER_CLOUD;
    }

    f32 row_polar_length = distance_from_surface + CENTER_RADIUS;

    Entity e1(ENEMY_RADIUS, PINK, 0.0f, row_polar_length, e_EntityKind::ENEMY, e_MovementKind::CIRCULAR, dir);
    enemies[0] = e1;

    i32 row_count = 0;
    i32 entities_in_row = 0;
    for(int i = 1; i < enemy_capacity; i++) {

        if (entities_in_row >= Row::rows_length[row_count]){
            row_count += 1;
            row_polar_length += 30;   
            entities_in_row = 0;
        }

        entities_in_row +=1;

        f32 radians = GetNextRadians(row_polar_length, ENEMY_RADIUS, ENEMY_RADIUS);

        Entity current_e (
            ENEMY_RADIUS, PINK, radians * entities_in_row, 
            row_polar_length, e_EntityKind::ENEMY, 
            e_MovementKind::CIRCULAR, dir
        );
        enemies[i] = current_e;
        
    }
}



void EnemyCloud::UpdateEnemies(f32 dt) {
    switch (action)
    {
        case WAITING: {
            if(delay >= E_TIME_STEP) {
                delay = 0.0f;
                action = MOVING;
                for(int i = 0; i < enemies.size(); i++)
                {
                    const Entity& e = enemies[i];
                    new_angle[i] = E_ANGLE_STEP + e.polar.rad;
                }
            } else {
                delay += dt;
            }
        } break;
        case MOVING: {
            for(int i = 0; i < enemies.size(); i++)
            {
                Entity& e = enemies[i];
                // f32 delta = new_angle[i] - e.polar.rad;
                // while (delta > PI)  delta -= 2.0f * PI;
                // while (delta < -PI) delta += 2.0f * PI;
                //
                // f32 target = e.polar.rad + delta;
                e.polar.rad = fLerp(e.polar.rad, new_angle[i], lerp_factor);
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


