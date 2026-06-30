#include "game.h"
#include "entity.h"
#include "raylib.h"
#include "resources.h"
#include "vstd/vmath.h"
#include "vstd/vrandom_gen.h"
#include "vstd/vlogger.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>


constexpr f32 EPSILON = 0.1f;
constexpr f32 PLAYER_MAX_SPEED = 4.0f;
constexpr f32 PLAYER_DASH_MAX_SPEED = 14.0f;
constexpr f32 PLAYER_DASH_LENGTH = 0.3f;
constexpr f32 PLAYER_DASH_COOLDOWN = 1.0f;
constexpr f32 PLAYER_ACCELERATION = 40.0f;
constexpr f32 ANIMATION_INCREMENT = 3.0f;

Game::Game(i32 w_width, i32 w_height, std::unique_ptr<ExperienceSystem> exp, std::unique_ptr<BuildingSystem> bs) :
    win_size(w_width, w_height)
{
    exp_sys = std::move(exp);
    build_sys = std::move(bs);

    scenario.tex = Resources::get_texture("planet");
    scenario.radius = CENTER_RADIUS;

    player.movement_kind = e_MovementKind::CIRCULAR;
    player.size = E_RADIUS * 2.0f;
    player.color = E_COLOR;
    player.polar.rad = PI * 0.5f;
    player.polar.length = scenario.radius + player.size;
    player.max_speed = PLAYER_MAX_SPEED;
    player.max_bullet_speed.length = BULLET_SPEED_OUTER;
    player.max_bullet_speed.rad = BULLET_SPEED_CIRCULAR;
    player.power_level = 1;
    player.health = 3;

    camera_state->target = {0, 0};
    camera_state->offset.x = w_width * 0.5f;
    camera_state->offset.y = w_height * 0.5f;
    camera_state->zoom = 1.6f;

    UpdateUpgradeRectangle(win_size);
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
    if (Absf32(player.speed_polar.rad) >= EPSILON){
        player.t += dt * Absf32(player.speed_polar.rad);
    }

    if (player.dashing) {
        player.dash_time -= dt;
    } else {
        if(player.accelerating)
            player.speed_polar.rad += PLAYER_ACCELERATION *  dt * (f32) player.direction;
        else player.speed_polar.rad *= FLOOR_FRICTION;

        player.speed_polar.rad = Clampf32(player.speed_polar.rad, -player.max_speed, player.max_speed);
        player.dash_cooldown -= dt;
    }
    
    // Makes movement equal independent of polar length.
    f32 delta = (player.speed_polar.rad / player.polar.length);
    player.polar.rad += delta;
    player.FixRad();


    if (player.dash_time <= 0) {
        player.dashing = false;
    }

}

void Game::UpdateOthers(f32 dt) {
    for (auto& bullet: bullets) {
        if(bullet.is_alive){
            bullet.Update(dt);
        }
    }
    for (auto& cloud: enemy_clouds) {
        if(cloud.is_alive)
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
    return (f32)sqrt(pow(v0.length, 2) + pow(v1.length, 2) - 2 * v0.length*v1.length * cos(v0.rad - v1.rad));
}

void Game::Collisions(f32 dt) {
    for (ExpParticle& particle: exp_sys->particles){
        if (!particle.active) continue;

        if (!particle.on_surface && scenario.radius + E_RADIUS * 0.5f >= particle.polar.length) {
            particle.on_surface = true;
            particle.lengths_lerp[0] = particle.polar.length;
            particle.lengths_lerp[1] = scenario.radius + 30.0f;
        }

        if (GetDistance(player.polar, particle.polar) < E_RADIUS){
            player.score += 1;
            particle.active = false;
            if (exp_sys->ShouldLevelUp(player.score)) {
                player.score = 0;
                state = e_GameState::LEVEL_UP;
                exp_sys->GetNewUpgrades();
            }
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
                        {bullet_pos.x, bullet_pos.y}, bullet.size, 
                        {enemy_pos.x, enemy_pos.y}, enemy.size
                    )){
                        i32 temp = bullet.damage;
                        bullet.damage -= enemy.health;
                        enemy.health -= temp;
                    
                        if (bullet.damage <= 0)
                            bullet.is_alive = false;
                        if (enemy.health <= 0){
                            enemy.is_alive = false;
                            enemy_clouds[enemy.cloud_idx].count-- ;
                            exp_sys->DropXP(enemy.polar, enemy.power_level);
                        }
                        break;
                    }
                }
            }
        } else if (bullet.team == e_Team::BAD_GUYS) {
            bool collide_planet = bullet.polar.length < scenario.radius;
            if (collide_planet) {
                bullet.is_alive = false;
            } else {
                fvec2 player_pos = PolarToCartesian(player.polar.length, player.polar.rad);
                if(CheckCollisionCircles(
                    {bullet_pos.x, bullet_pos.y}, bullet.size, 
                    {player_pos.x, player_pos.y}, player.size
                )){
                    player.health-=1;
                    if (player.health <= 0)
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
    DrawOnPolar(scenario.tex, {0, 0}, scenario.radius * 2.0f + 40.0f, scenario.radius * 2.0f + 40.0f);   

    //Player
    fvec2 player_pos = PolarToCartesian(player.polar.length, player.polar.rad);

    DrawCircle((i32)player_pos.x, (i32)player_pos.y, player.size, player.color);
    
    for (const auto& bullet: bullets){
        if(bullet.is_alive){
            fvec2 bullet_pos = PolarToCartesian(bullet.polar.length, bullet.polar.rad);
            DrawCircle((i32)bullet_pos.x, (i32)bullet_pos.y, bullet.size, bullet.color);
        }
    }


    for (const auto& cloud: enemy_clouds){
        for (const auto& enemy: cloud.enemies){
            if(enemy.is_alive){
                fvec2 enemy_pos = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                DrawCircle((i32)enemy_pos.x, (i32)enemy_pos.y, enemy.size, enemy.color);
            }
        }
    }
    
    for (const auto& particle: exp_sys->particles){
        if (particle.active){
            fvec2 particle_pos = PolarToCartesian(particle.polar.length, particle.polar.rad);
            DrawCircle((i32)particle_pos.x, (i32)particle_pos.y, EXP_RADIUS, YELLOW);
        }
    }

}

size_t GetClickedOption(std::array<RectangleReac, 4> rectangles, e_UIMode mode){
    i32 n = mode == e_UIMode::THREE ? 3 : 4;
    size_t return_value = -1;
    Vector2 mouse_pos = GetMousePosition();
    for(size_t i = 0; i < n; i++){
        auto& rec = rectangles[i];
        if (CheckCollisionPointRec(mouse_pos, rec)){
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
                return_value = i;
            }
            rec.hovered = true;
        } else {
            rec.hovered = false;
        }
    }

    return return_value;
}

void Game::ProcessInput(f32 dt) {
    switch (state) {
        case e_GameState::SELECT_BUILDING: {
            size_t id = GetClickedOption(UIRectangles, UIMode); 
            if (id != -1){
                player.building_points -= 1 ;
                player.carrying = e_BuildingKind(id + 1);
                state = e_GameState::PLAY;
            }
        } break;
        case e_GameState::LEVEL_UP: {
            size_t id = GetClickedOption(UIRectangles, UIMode); 
            if (id != -1){
                exp_sys->NewUpgrades[id].command(player);
                state = e_GameState::PLAY;
            }
        } break;
        
        case e_GameState::PAUSE: {} break;
        case e_GameState::PLAY: {
            // ZOOM
            f32 mouse_wheel = GetMouseWheelMove();   
            if(mouse_wheel != 0){
                camera_state->zoom += mouse_wheel * dt;
            }
            if(IsKeyDown(KEY_M))
                BABUSKA += 40.0f * dt;
            if(IsKeyDown(KEY_N))
                BABUSKA -= 40.0f * dt;
            //DASH
            if(IsKeyDown(KEY_D) && player.direction != 0 && player.dash_cooldown <= 0) {
                player.dash_cooldown = PLAYER_DASH_COOLDOWN;
                player.dash_time = PLAYER_DASH_LENGTH;
                player.speed_polar.rad = PLAYER_DASH_MAX_SPEED * player.direction;
                player.dashing = true;
            }
                
            if(IsKeyDown(KEY_LEFT)) {
                player.direction = 1; 
                player.accelerating = true;
            }
            else if(IsKeyDown(KEY_RIGHT)) {
                player.direction = -1; 
                player.accelerating = true;
            }
            else player.accelerating = false;
            

            // SHOOT
            if (IsKeyPressed(KEY_SPACE))
                bullets[bullet_count++] = player.ShootV();
            if (IsKeyPressed(KEY_X))
                bullets[bullet_count++] = player.ShootH(-1);
            if (IsKeyPressed(KEY_Z))
                bullets[bullet_count++] = player.ShootH(1);

            if (IsKeyPressed(KEY_D) && player.IsCarrying()){
                printf("Player radian position %02f Index %d\n", player.polar.rad, build_sys->GetIndexGivenRadians(player.polar.rad));
                build_sys->Create(player.carrying, build_sys->GetIndexGivenRadians(player.polar.rad));
            }

        } break;
    }
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

        game.NewEnemyCloud(num_enemies, distance + game.scenario.radius, direction);

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


bool ExperienceSystem::ShouldLevelUp(i32 current_score){
    return current_score >= EXP_PER_LEVEL[level]? true : false;
} 

void Game::UpdateRectangle(vec2 win_size) {
    switch (UIMode) {
        case e_UIMode::THREE:{
            UpdateUpgradeRectangle(win_size);
        }break;
        case e_UIMode::FOUR:{
            UpdateBuildingRectangle(win_size);
        }break;
        case e_UIMode::NIL:
        case e_UIMode::COUNT:{}break;
    };
}


void Game::UpdateBuildingRectangle(vec2 win_size) {
    static i32 horizontal_padding = 20; 
    static i32 top_padding = 20; 
    static i32 bottom_padding = 10;
    static i32 option_padding = 10;

    static i32 option_width = i32(((i32)win_size.x - (horizontal_padding * 2 + option_padding * 2)) / 2.0f);
    static i32 option_height = (i32)win_size.y - (top_padding + bottom_padding);

    i32 pos_x = horizontal_padding;
    i32 pos_y = top_padding;


    for(int i = 0; i < 2; i++){      
        for(int j = 0; j < 2; j++){
            UIRectangles[i + j] = Rectangle{(f32)pos_x, (f32)pos_y, (f32)option_width, (f32)option_height};
            UIRectangles[i + j].hovered = false;
            pos_x += option_width + option_padding;
        }
        pos_y += option_height + bottom_padding;
    }

}

void Game::UpdateUpgradeRectangle(vec2 win_size) {

    static i32 horizontal_padding = 20; 
    static i32 top_padding = 20; 
    static i32 bottom_padding = 10;
    static i32 upgrade_padding = 10;

    static i32 upgrade_width = i32(((i32)win_size.x - (horizontal_padding * 2 + upgrade_padding * 2)) / 3.0f);
    static i32 upgrade_height = (i32)win_size.y - (top_padding + bottom_padding);

    i32 pos_x = horizontal_padding;
    i32 pos_y = top_padding;

    for(int i = 0; i < 3; i++){      
        UIRectangles[i] = Rectangle{(f32)pos_x, (f32)pos_y, (f32)upgrade_width, (f32)upgrade_height};
        UIRectangles[i].hovered = false;
        pos_x += upgrade_width + upgrade_padding;
    }
}


void ExperienceSystem::DrawUpgrades(std::array<RectangleReac, 4> rectangles, vec2 win_size){
    
    static i32 title_top_padding = 8;
    static i32 title_left_padding = 10;

    for(int i = 0; i < 3; i++){
        const auto& rec = rectangles[i];
        Color c = rec.hovered ? YELLOW : RAYWHITE;
        DrawRectangleRec(rec, c);
        DrawRectangleLines((i32)rec.x, (i32)rec.y, (i32)rec.width, (i32)rec.height, BLACK);

        const Texture2D& tex = NewUpgrades[i].image;
        f32 min_size = fmin(rec.width - 2.0f * title_left_padding, (f32)tex.height);
        printf("%02f\n", min_size);
        Rectangle dst = {
            rec.x + title_left_padding, rec.y + title_top_padding, 
            min_size, min_size
        };

        DrawTexturePro(
            tex, {0, 0, (f32)tex.width, (f32)tex.height}, dst, 
            {0,0}, 0.0f, WHITE
        );

        DrawText(NewUpgrades[i].name.c_str(), i32(rec.x + title_left_padding), i32(rec.y + title_top_padding + dst.height), 20, BLACK);
    }
}

void ExperienceSystem::GetNewUpgrades() {
    for(int i = 0; i < 3; i++){
        while (true) {
            Upgrade new_upgrade = pbs::upgrades::Get((e_UpgradeKind) rng::randi32(0, (i32)e_UpgradeKind::COUNT - 1));
            bool add = true;
            for(const auto& upgrade: NewUpgrades){
                if (upgrade.kind == new_upgrade.kind){
                    add = false;
                    break;
                }
            }
            if (add){
                NewUpgrades[i] = new_upgrade;
                break;
            }
        }
    }
}

void ExperienceSystem::DropXP(fvec2Polar polar, i32 power_level){
    i32 n_exp_particles = power_level;
    for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++) {
        ExpParticle& particle = particles[i];
        if (!particle.active) {
            particle.on_surface = false;
            particle.active = true;
            particle.polar.length = polar.length;
            particle.polar.rad = polar.rad;
            particle.speed.length = rng::randf32(-3.0f, 1.0f);
            particle.speed.rad = rng::randf32(-0.2f, 0.2f);

            n_exp_particles -= 1;
            if (n_exp_particles <= 0) break;
        }
    }
}



void Game::DrawUI(){
    char buff[30];
    const f32 v_step = 40.0f;
    f32 v_start = 20.0f;
    const f32 h_start = 20.0f;
    const struct {
        std::string text;
        f32 value;
    } Frases[] = {
        {"Score: %02f\n", f32(player.score)},
        {"Bullet speed: %02f\n", player.max_speed},
        {"Power: %02f\n", f32(player.power_level)},
        {"BABUSKA: %02f\n", f32(BABUSKA)},
    };
    
    for(const auto & s: Frases){
        sprintf_s(buff, s.text.c_str(), s.value);
        DrawText(buff, (i32)h_start, (i32)v_start, 30, BLACK);                         
        v_start += v_step;
    }

    if (state == e_GameState::LEVEL_UP){
        exp_sys->DrawUpgrades(UIRectangles, win_size);
    } else if (state == e_GameState::SELECT_BUILDING){
        build_sys->DrawOptions(UIRectangles, win_size);
    }
}

void Game::DrawEntities(){
    Texture2D enemy_texture = Resources::get_texture("enemy");

    for(const auto& cloud: enemy_clouds) {
        for (const auto& enemy: cloud.enemies) {
            if (!enemy.is_alive) continue;
            f32 size = enemy.size * 2.0f;
            DrawOnPolar(enemy_texture, enemy.polar, size, size);
        }
    }

    Texture2D pj_texture = Resources::get_texture("character");
    
    f32 el_sin = (f32)sin(player.t) * ANIMATION_INCREMENT;
    const f32 draw_w = player.size * 2.0f + el_sin;
    const f32 draw_h = player.size * 2.0f - el_sin;

    DrawOnPolar(pj_texture, player.polar, draw_w, draw_h);
}


void DrawOnPolar(Texture2D tex, fvec2Polar position, f32 width, f32 height) {
    fvec2 pos_world = PolarToCartesian(position.length, position.rad);
    f32 rot = (-position.rad * RAD2DEG) + 90.0f;

    DrawTexturePro(
        tex, {0, 0, (f32)tex.width, (f32)tex.height},
        {pos_world.x, pos_world.y, width, height},
        {width * 0.5f, height * 0.5f}, 
        rot, WHITE
    );
}


Building BuildingSystem::Destroy(size_t position_on_world){
    auto b = buildings.at(position_on_world);
    if(b.kind != e_BuildingKind::NIL){
        buildings.at(position_on_world) = Building();
        if (b.brother_idx != -1) {
            buildings.at(b.brother_idx) = Building();
        }
    }

    return b;
}


void BuildingSystem::Create(e_BuildingKind idx, size_t position_on_world){
    auto& b = buildings.at(position_on_world);
    if(b.kind == e_BuildingKind::NIL){
        b = pbs::buildings::Get(idx);
    }
}

void BuildingSystem::Update(){
    for(auto& b: buildings){
        if(b.kind != e_BuildingKind::NIL){
            // switch (condition) {
            //
            // }
            //
            //
            // b.Execute();
        }
    };
}

Texture2D BuildingSystem::__get_texture(e_BuildingKind kind){
    switch (kind) {
        case e_BuildingKind::DRILL_STATION:{return Resources::get_texture("building_drill_station");}break;
        case e_BuildingKind::THORN_FIELD:{return Resources::get_texture("building_drill_station");}break;
        case e_BuildingKind::V_SHOT:{return Resources::get_texture("building_drill_station");}break;
        case e_BuildingKind::COUNT:
        case e_BuildingKind::NIL:
        default:
            V_LOG_ERROR("BUILDING WITH NIL/COUT KIND"); return Resources::get_texture("fallback");
    }
    
}

std::string_view BuildingSystem::__get_name(e_BuildingKind kind){
    switch (kind) {
        case e_BuildingKind::DRILL_STATION:{return "Drill Station\n";}break;
        case e_BuildingKind::THORN_FIELD:{return "Thorn Field\n";}break;
        case e_BuildingKind::V_SHOT:{return "Laser Tower\n";}break;
        case e_BuildingKind::COUNT:
        case e_BuildingKind::NIL:
        default:
            V_LOG_ERROR("BUILDING WITH NIL/COUT KIND"); 
            return "FALLBACK\n";
    }
}
void BuildingSystem::DrawBuildings(){
    for(auto& b: buildings){
        if(b.kind != e_BuildingKind::NIL){
            f32 size = b.size * 2.0f;
            DrawOnPolar(__get_texture(b.kind), b.polar, size, size);
        }
    };
}

void BuildingSystem::DrawOptions(std::array<RectangleReac, 4> rectangles, vec2 win_size){
    static i32 title_top_padding = 8;
    static i32 title_left_padding = 10;

    for(int i = 0; i < 4; i++){
        const auto& rec = rectangles[i];
        Color c = rec.hovered ? YELLOW : RAYWHITE;
        DrawRectangleRec(rec, c);
        DrawRectangleLines((i32)rec.x, (i32)rec.y, (i32)rec.width, (i32)rec.height, BLACK);

        e_BuildingKind building_id = e_BuildingKind(i + 1);

        const Texture2D& tex = __get_texture(building_id);
        f32 min_size = fmin(rec.width - 2.0f * title_left_padding, (f32)tex.height);
        Rectangle dst = {
            rec.x + title_left_padding, rec.y + title_top_padding, 
            min_size, min_size
        };

        DrawTexturePro(
            tex, {0, 0, (f32)tex.width, (f32)tex.height}, dst, 
            {0,0}, 0.0f, WHITE
        );

        DrawText(__get_name(building_id).data(), i32(rec.x + title_left_padding), i32(rec.y + title_top_padding + dst.height), 20, BLACK);
    }
}


i32 BuildingSystem::GetIndexGivenRadians(f32 rad){
    return i32(rad / BUILDING_SIZE);
}
