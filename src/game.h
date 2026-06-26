
#include "entity.h"
#include "vstd/vtypes.h"
#include <array>
#include <cstddef>
#include <raylib.h>
#include <vector>
#include <memory>


struct Game;

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

struct RectangleReac : public Rectangle{
    bool hovered;

    RectangleReac& operator=(const Rectangle& r) {
        Rectangle::operator=(r);
        return *this;
    }
};

struct ExperienceSystem {
    inline const static i32 MAX_LEVEL = 6;
    inline static std::array<i32, MAX_LEVEL> EXP_PER_LEVEL = { 10, 12, 23, 27, 30, 31 };

    i32 exp = 0;
    i32 level = 0;

    bool adjust = false;
    std::array<Upgrade, MAX_UPGRADES> PlayerUpgrades;
    std::array<Upgrade, 3> NewUpgrades;
    std::array<ExpParticle, MAX_NUM_EXP_PARTICLES> particles;
    std::array<RectangleReac, 3> UpgradeRectangle;

    ExperienceSystem() = default;
    void Update(f32 dt);
    bool ShouldLevelUp(i32 current_score); 
    void GetNewUpgrades();
    void UpdateUpgradeRectangle(vec2 win_size);
    void DrawUpgrades(vec2 win_size);
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
    Texture2D tex;
    f32 radius;
};

enum class e_GameState{
    PLAY, 
    PAUSE, 
    LEVEL_UP
};

struct Game {
    f32 BABUSKA = 0.0f;

    
    e_GameState state = e_GameState::PLAY;
    vec2 win_size = {};
    Player player;
    std::vector<EnemyCloud> enemy_clouds;
    std::array<Bullet, NUM_BULLETS> bullets;

    size_t bullet_count = 0;

    CameraState camera_state = {};

    Scenario scenario = {};
    
    std::unique_ptr<ExperienceSystem> exp_sys;
    Game(i32 w_width, i32 w_height, std::unique_ptr<ExperienceSystem> exp_sys);

    void UpdatePlayer(f32 dt);
    void UpdateOthers(f32 dt);
    void UpdateCamera(f32 dt);
    void Collisions(f32 dt);
    void Draw();
    void DrawEntities();
    void DrawUI();
    void ProcessInput(f32 dt);
    void NewEnemyCloud(size_t num_enemies, f32 distance_from_surface, i32 direction);
};

struct SpawSystem {
    f32 till_next_cloud = 0.0f;
    Game& game;
    void Update(f32 dt);
    SpawSystem(Game& game);
};

enum class e_BuildingKind {
    NIL,
    V_SHOT,         // VERTICAL SHOT
    THORN_FIELD,    // TAKES HEALTH FROM WHOEVER STEPS ON IT
    DRILL_STATION,  // ALLOWS YOU TO GO TO DRILL STATION ON OPOSITE SIDE.
    COUNT
};

struct Building {
    fvec2Polar polar;
    e_BuildingKind kind;   

    i32 index;
    i32 brother_idx; // No problem cause there is no swaps on our ds.
    Building() = default;
    Building(fvec2Polar, e_BuildingKind, i32, i32);
    void Execute();
};


constexpr i32 BUILDING_MAX_COUNT = 1024;
struct BuildingSystem {
    std::array<Building, BUILDING_MAX_COUNT> buildings;
    std::array<i16, BUILDING_MAX_COUNT> __free_list;

    f32 timer;
    BuildingSystem() = default;
    void Update();
};

void DrawOnPolar(Texture2D tex, fvec2Polar position, f32 width, f32 height);
