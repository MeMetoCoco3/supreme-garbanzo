#include "game.h"
#include "entity.h"
#include "vstd/vmath.h"
#include "vstd/vrandom_gen.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <utility>


constexpr f32 PLAYER_MAX_SPEED = 10.0f;
constexpr f32 PLAYER_DASH_MAX_SPEED = 14.0f;
constexpr f32 PLAYER_DASH_LENGTH = 0.3f;
constexpr f32 PLAYER_DASH_COOLDOWN = 1.0f;
constexpr f32 PLAYER_ACCELERATION = 40.0f;

Game::Game(i32 w_width, i32 w_height, std::unique_ptr<ExperienceSystem> exp) :
    win_size(w_width, w_height)
{
    exp_sys = std::move(exp);

    scenario.polar.length = CENTER_RADIUS;

    player.movement_kind = e_MovementKind::CIRCULAR;
    player.radius = E_RADIUS;
    player.color = E_COLOR;
    player.polar.rad = PI * 0.5f;
    player.polar.length = scenario.polar.length + E_RADIUS;

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
        else player.speed_polar.rad *= FLOOR_FRICTION;

        player.speed_polar.rad = Clampf32(player.speed_polar.rad, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);

        player.dash_cooldown -= dt;
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
        cloud.Shoot(bullets, bullet_count, dt);
    }
    for (auto& particle: exp_sys->particles) {
        particle.Update(dt);
    }
}

void Game::UpdateCamera(f32 dt) {
    fvec2Polar player_on_polar = player.polar;
    player_on_polar.length += 50.0f;

    fvec2 player_pos =  PolarToCartesian(player_on_polar.length, player_on_polar.rad);
    camera_state->target.x = player_pos.x;
    camera_state->target.y = player_pos.y;
    camera_state->rotation = (player_on_polar.rad * RAD2DEG) - 90.0f;
}

f32 GetDistance(fvec2Polar v0, fvec2Polar v1){
    return (f32)sqrt(pow(v0.length, 2) + pow(v1.length, 2) - 2*v0.length*v1.length * cos(v0.rad - v1.rad));
}

void Game::Collisions(f32 dt) {
    for (ExpParticle& particle: exp_sys->particles){
        if (!particle.active) continue;

        if (!particle.on_surface && scenario.polar.length + E_RADIUS * 0.5f >= particle.polar.length) {
            printf("HIT!\n");
            particle.on_surface = true;
            particle.lengths_lerp[0] = particle.polar.length;
            particle.lengths_lerp[1] = scenario.polar.length + 30.0f;
        }

        if (GetDistance(player.polar, particle.polar) < E_RADIUS){
            player.score += 1;
            particle.active = false;
        }
    }

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

                        // DROP EXP
                        i32 n_exp_particles = enemy.power_level;
                        for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++) {
                            ExpParticle& particle = exp_sys->particles[i];
                            if (!particle.active) {
                                particle.active = true;
                                particle.polar.length = enemy.polar.length;
                                particle.polar.rad = enemy.polar.rad;
                                particle.speed.length = rng::randf32(-3.0f, 1.0f);
                                particle.speed.rad = rng::randf32(-0.2f, 0.2f);

                                printf("%d SL: %02f SR: %02f\n", n_exp_particles, particle.speed.length, particle.speed.rad);
                                n_exp_particles -= 1;
                                if (n_exp_particles <= 0) break;
                            }
                        }

                        break;
                    }
                }
            }
        } else if (bullet.team == e_Team::BAD_GUYS) {
            bool collide_planet = bullet.polar.length < scenario.polar.length;
            if (collide_planet) {
                bullet.is_alive = false;
            } else {
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
}

constexpr i32 EXP_RADIUS = 5;

void Game::Draw() {
    ClearBackground(RAYWHITE);

    // World
    DrawCircle(0, 0, scenario.polar.length, RED);
    DrawCircleLines(0, 0, scenario.polar.length, BLACK);
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
    
    for (const auto& particle: exp_sys->particles){
        if (particle.active){
            fvec2 particle_pos = PolarToCartesian(particle.polar.length, particle.polar.rad);
            printf("%02f, %02f\n", particle_pos.x, particle_pos.y);
            DrawCircle((i32)particle_pos.x, (i32)particle_pos.y, EXP_RADIUS, YELLOW);
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
        bullets[bullet_count++] = player.Shoot(e_MovementKind::OUTER, 0, BULLET_SPEED_OUTER, e_Team::GOOD_GUYS, 1); 
    if (IsKeyPressed(KEY_X))
        bullets[bullet_count++] = player.Shoot(e_MovementKind::CIRCULAR, BULLET_SPEED_CIRCULAR, 0,  e_Team::GOOD_GUYS, -1);
    if (IsKeyPressed(KEY_Z))
        bullets[bullet_count++] = player.Shoot(e_MovementKind::CIRCULAR, BULLET_SPEED_CIRCULAR, 0, e_Team::GOOD_GUYS, 1);
}

constexpr f32 TIME_BETWEEN_CLOUDS = 100000000.0f;
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

        game.NewEnemyCloud(num_enemies, distance + game.scenario.polar.length, direction);

    } else {
        till_next_cloud -= dt;
    }
}

constexpr f32 GRAVITY_ACCELERATION = 3.3f;
void ExpParticle::Update(f32 dt) {
    if (on_surface){
        polar.length = fLerp(lengths_lerp[0], lengths_lerp[1], t);
        t += dt;
        if (t >= 1.0f) {
            t = 0;
            std::swap(lengths_lerp[0], lengths_lerp[1]);
        }
    } else {
        polar.length += speed.length * dt;
        polar.rad += speed.rad * dt;

        speed.length -= GRAVITY_ACCELERATION;

    }
    
}

