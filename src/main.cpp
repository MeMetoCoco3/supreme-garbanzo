#include <memory>
#include <raylib.h>
#include <math.h>
#include "entity.h"
#include "game.h"
#include "resources.h"

constexpr f32 ANIMATION_INCREMENT = 3.0f;
constexpr i32 WIN_WIDTH = 800;
constexpr i32 WIN_HEIGHT = 800;
constexpr auto TITLE = "ROTATOS\n";


constexpr int balls_around = 12;

int main(void){

    InitWindow(WIN_WIDTH, WIN_HEIGHT, TITLE);
    Resources::Init();
    SetTargetFPS(60);
    
    std::unique_ptr<ExperienceSystem> exp_sys = std::make_unique<ExperienceSystem>();
    Game Game(WIN_WIDTH, WIN_HEIGHT, std::move(exp_sys));
    f32 dt = 1.0f/60.0f;

    SpawSystem spawn_sys(Game);
    // Add Debug Balls on World surface.
    fvec2Polar d_balls_around[balls_around];
    int step = 360 / balls_around;
    for(int i = 0;  i < balls_around; i++)
    {
        d_balls_around[i].length = Game.scenario.polar.length;
        d_balls_around[i].rad = f32(step * i);
    }


    while(!WindowShouldClose())
    {
        Game.ProcessInput(dt);
        if (Game.state == e_GameState::PLAY){
            Game.UpdatePlayer(dt);
            Game.UpdateOthers(dt);
            Game.Collisions(dt);
            Game.UpdateCamera(dt);
            spawn_sys.Update(dt);
        }
        

        BeginDrawing();
            BeginMode2D(Game.camera_state);
            Game.Draw();
            for (auto ball: d_balls_around){
                fvec2 p = PolarToCartesian(ball.length, ball.rad * DEG2RAD);
                DrawCircle((i32)p.x, (i32)p.y, 4, YELLOW);
            }
            EndMode2D();
            char buff[5];
            sprintf_s(buff, "%d\n", Game.player.score);
            DrawText(buff, 20, 20, 50, BLACK);
                
            Texture2D enemy_texture = Resources::get_texture("enemy");
            for(const auto& cloud: Game.enemy_clouds) {
                for (const auto& enemy: cloud.enemies) {
                if (!enemy.is_alive) continue;
                fvec2 pos_world = PolarToCartesian(enemy.polar.length, enemy.polar.rad);
                Vector2 pos_window = GetWorldToScreen2D({pos_world.x, pos_world.y}, Game.camera_state.camera);
                // f32 el_sin = (f32)sin(Game.player.t) * ANIMATION_INCREMENT;
                    DrawTexturePro(
                        enemy_texture, {0, 0, (f32)enemy_texture.width, (f32)enemy_texture.height},
                        {pos_window.x, pos_window.y, 50.0f, 50.0f},
                        {50.0f * 0.5f, 50.0f * 0.5f}, 
                        -(RAD2DEG * (enemy.polar.rad - Game.player.polar.rad)), WHITE);
                }
            }

            Texture2D pj_texture = Resources::get_texture("character");

            fvec2 player_pos_world = PolarToCartesian(Game.player.polar.length, Game.player.polar.rad);
            Vector2 player_pos_window = GetWorldToScreen2D(
                    {player_pos_world.x, player_pos_world.y}, Game.camera_state.camera);
            f32 el_sin = (f32)sin(Game.player.t) * ANIMATION_INCREMENT;


            const f32 draw_w = (100.0f * Game.camera_state->zoom) + el_sin;
            const f32 draw_h = (100.0f * Game.camera_state->zoom) - el_sin;

            f32 corner_x = roundf(player_pos_window.x - draw_w * 0.5f);
            f32 corner_y = roundf(player_pos_window.y - draw_h * 0.5f) - 30.0f;

            DrawTexturePro(
                pj_texture,
                {0, 0, (f32)pj_texture.width, (f32)pj_texture.height},
                {corner_x, corner_y, draw_w, draw_h},
                {0, 0},          
                0.0f, WHITE
            );

            if (Game.state == e_GameState::LEVEL_UP){
                Game.exp_sys->DrawUpgrades(Game.win_size);
            }


        EndDrawing();
    }
    return 0;
}


