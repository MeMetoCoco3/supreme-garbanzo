#include <raylib.h>
#include <math.h>
#include "entity.h"
#include "game.h"

constexpr i32 WIN_WIDTH = 800;
constexpr i32 WIN_HEIGHT = 800;
constexpr auto TITLE = "ROTATOS\n";


constexpr int balls_around = 12;

int main(void){

    InitWindow(WIN_WIDTH, WIN_HEIGHT, TITLE);
    SetTargetFPS(60);
    
    Game Game(WIN_WIDTH, WIN_HEIGHT);
    f32 dt = 1.0f/60.0f;


    SpawSystem spawn_sys(Game);
    // Add Debug Balls on World surface.
    fvec2Polar d_balls_around[balls_around];
    int step = 360 / balls_around;
    for(int i = 0;  i < balls_around; i++)
    {
        d_balls_around[i].length = CENTER_RADIUS;
        d_balls_around[i].rad = f32(step * i);
    }


    while(!WindowShouldClose())
    {
        Game.ProcessInput(dt);
        Game.UpdatePlayer(dt);
        Game.UpdateOthers(dt);
        Game.Collisions(dt);
        Game.UpdateCamera(dt);
        spawn_sys.Update(dt);

        BeginDrawing();
            BeginMode2D(Game.camera_state);
            Game.Draw();
            for (auto ball: d_balls_around){
                fvec2 p = PolarToCartesian(ball.length, ball.rad * DEG2RAD);
                DrawCircle((i32)p.x, (i32)p.y, 4, YELLOW);
            }
            EndMode2D();
        EndDrawing();
    }
    return 0;
}


