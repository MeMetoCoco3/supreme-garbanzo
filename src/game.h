
#include "entity.h"
#include "vstd/vtypes.h"
#include <cstddef>
#include <raylib.h>
#include <vector>

constexpr i32 NUM_BULLETS = 100;

struct CameraState {
    enum State {
        FOLLOW,
        STATIC
    } state = STATIC;
    Camera2D camera = {};
    
    Camera2D* operator->() { return &camera; };
    operator Camera2D&() { return camera; };
}; 



struct Game {
    vec2 win_size = {};

    Entity player;
    std::vector<EnemyCloud> enemy_clouds;
    std::array<Bullet, NUM_BULLETS> bullets;

    size_t bullet_count = 0;

    CameraState camera_state = {};

    Game(i32 w_width, i32 w_height);
    void UpdatePlayer(f32 dt);
    void UpdateOthers(f32 dt);
    void UpdateCamera(f32 dt);
    void Collisions(f32 dt);
    void Draw();
    void ProcessInput(f32 dt);
    void NewEnemyCloud(size_t num_enemies, f32 distance_from_surface, i32 direction);
};

struct SpawSystem {
    f32 till_next_cloud = 0.0f;
    Game& game;
    void Update(f32 dt);
    SpawSystem(Game& game);
};
