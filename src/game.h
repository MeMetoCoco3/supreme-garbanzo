
#include "entity.h"
#include "vstd/vtypes.h"
#include <cstddef>
#include <raylib.h>
#include <vector>
#include <memory>

struct ExpParticle {
    fvec2Polar polar = {0}; 
    fvec2Polar speed = {0};

    // Animation
    f32 lengths_lerp[2] = {0, 0};
    f32 t = 0;
    bool t_direction = 0;
    bool active = false;
    bool on_surface = false;
    void Update(f32 dt);
    ExpParticle() = default;
};

constexpr size_t MAX_NUM_EXP_PARTICLES = 1024;

struct ExperienceSystem {
    void Update(f32 dt);
    std::array<ExpParticle, MAX_NUM_EXP_PARTICLES> particles;
    ExperienceSystem() = default;
};

struct CameraState {
    enum State {
        FOLLOW,
        STATIC
    } state = STATIC;
    Camera2D camera = {};
    
    Camera2D* operator->() { return &camera; };
    operator Camera2D&() { return camera; };
}; 

struct Scenario {
    fvec2Polar polar;  
};

struct Game {
    vec2 win_size = {};
    Entity player;
    std::vector<EnemyCloud> enemy_clouds;
    std::array<Bullet, NUM_BULLETS> bullets;

    size_t bullet_count = 0;

    CameraState camera_state = {};

    Scenario scenario = {0};
    
    std::unique_ptr<ExperienceSystem> exp_sys;
    Game(i32 w_width, i32 w_height, std::unique_ptr<ExperienceSystem> exp_sys);

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

