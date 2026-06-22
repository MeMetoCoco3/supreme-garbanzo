#pragma once

#include <raylib.h>
#include "vstd/vtypes.h"
#include <unordered_map>
#include "sprite.h"


namespace Resources {

    static constexpr auto TexturePath = RESOURCES_PATH "textures/";
    static constexpr auto MusicPath = RESOURCES_PATH "musics/";
    static constexpr auto FxPath = RESOURCES_PATH "fxs/";
    static constexpr auto ScenesPath = RESOURCES_PATH "scenes/";

    constexpr u32 MAX_NUM_SCENES = 50;
    constexpr u32 MAX_NUM_TEXTURES = 50;
    constexpr u32 MAX_NUM_SPRITES = 50;
    constexpr u32 MAX_NUM_SONGS = 50;
    constexpr u32 MAX_NUM_FX = 50;

    inline int count_scenes = 0;
    inline std::string ScenesNames[MAX_NUM_SCENES];
    inline int count_textures = 0;
    inline Texture2D Textures[MAX_NUM_TEXTURES]; 
    inline int count_sounds = 0;
    inline Sound Sounds[MAX_NUM_FX];
    inline int count_musics = 0;
    inline Music Musics[MAX_NUM_SONGS];

    inline s_Sprite Sprites[MAX_NUM_SPRITES]; 
    
    inline std::unordered_map<std::string, int> file_to_id;

    void Init(); 

    void load_music(void);
    void load_fx(void);
    void load_textures(void);
    void load_scenes(void);

    Texture2D get_texture(const char* name);
    Music get_music(const char* name);
    Sound get_fx(const char* name);
    void create_sprites(void);
}

