#include "resources.h"
#include "vstd/vlogger.h"
#include "vstd/file_reader.h"
#include "filesystem"

using namespace Resources;

static constexpr auto TexturePath = RESOURCES_PATH "textures/";
static constexpr auto MusicPath = RESOURCES_PATH "musics/";
static constexpr auto FxPath = RESOURCES_PATH "fxs/";
static constexpr auto ScenesPath = RESOURCES_PATH "scenes/";


Texture2D Resources::get_texture(const char* name) {
    auto it = file_to_id.find(name);
    if (it == file_to_id.end())
    {
        V_LOG_WARN("TEXTURE NOT FOUND: %s USING FALLBACK.\n", name);
        return Textures[0];
    }
    return Textures[it->second];
}


Music Resources::get_music(const char* name) {
    return Musics[file_to_id.at(name)];
}

Sound Resources::get_fx(const char* name) {
    return Sounds[file_to_id.at(name)];
}
void Resources::load_music(void) {
    if (count_musics > 0)
    {
        // TODO: UNLOAD
        count_musics = 0;
    }
        
    if(!DirectoryExists(MusicPath)) {
        V_LOG_ERROR("DIRECTORY %s DOES NOT EXIST\n", MusicPath);
    } else {
        FilePathList files = LoadDirectoryFiles(MusicPath);
        
        for (size_t i = 0; i < files.count; i++)
        {
            std::string file_path = remove_double_slash(files.paths[i]);
            Music music = LoadMusicStream(file_path.c_str());
            if (music.stream.buffer){
                V_LOG_INFO("MUSIC STREAM LOADED: %s\n", file_path.c_str());

                Musics[count_musics] = music; 
                std::string file_name = std::string(GetFileNameWithoutExt(file_path.c_str()));
                file_to_id.insert({file_name, count_musics});
                count_musics += 1;
            }
        }
    }
}

void Resources::load_scenes(void) {
    count_scenes = 0;
    if(!DirectoryExists(ScenesPath)){
        V_LOG_ERROR("DIRECTORY %s DOES NOT EXIST\n", ScenesPath);
    } else {
        FilePathList files = LoadDirectoryFiles(ScenesPath);
        
        for (size_t i = 0; i < files.count; i++)
        {
            std::string file_path = remove_double_slash(files.paths[i]);
            const auto file_extension = GetFileExtension(file_path.c_str());
            if (strcmp(file_extension, ".json") == 0){
                V_LOG_INFO("SCENE LOADED: %s\n", file_path.c_str());

                std::string file_name = std::string(GetFileName(file_path.c_str()));
                ScenesNames[count_scenes] = file_name; 
                count_scenes += 1;
            }
        }
    }
};

void Resources::load_fx(void) {
    if (count_sounds > 0)
    {
        //TODO: UNLOAD
        count_sounds = 0;
    }
    if(!DirectoryExists(FxPath)){
        V_LOG_ERROR("DIRECTORY %s DOES NOT EXIST\n", FxPath);
    } else {
        FilePathList files = LoadDirectoryFiles(FxPath);
        
        for (size_t i = 0; i < files.count; i++)
        {
            std::string file_path = remove_double_slash(files.paths[i]);
            Sound sound = LoadSound(file_path.c_str());
            if (sound.stream.buffer){
                V_LOG_INFO("FX LOADED: %s\n", file_path.c_str());
                Sounds[count_sounds] = sound; 
                std::string file_name = std::string(GetFileNameWithoutExt(file_path.c_str()));
                file_to_id.insert({file_name, count_sounds});
                count_sounds += 1;
            }
        }
    }
}


void Resources::load_textures(void) {
    if (count_textures > 0)
    {
        //TODO: UNLOAD
        count_textures = 0;
    }
    if(!DirectoryExists(TexturePath)){
        V_LOG_ERROR("DIRECTORY %s DOES NOT EXIST\n", FxPath);
    } else {
        FilePathList files = LoadDirectoryFiles(TexturePath);
        
        for (size_t i = 0; i < files.count; i++)
        {
            if (std::filesystem::is_directory({files.paths[i]})) continue;

            std::string file_path = remove_double_slash(files.paths[i]);
            if(is_correct_extension(file_path.c_str(), {".jpg", ".png"}))
            {
                Texture2D texture = LoadTexture(file_path.c_str());
                if (texture.id != 0){
                    V_LOG_INFO("TEXTURE LOADED: %s\n", file_path.c_str());
                    
                    Textures[count_textures] = texture; 
                    std::string file_name = std::string(GetFileNameWithoutExt(file_path.c_str()));
                    file_to_id.insert({file_name, count_textures});
                    count_textures += 1;

                }
            }
        }
    }
}


enum SPRITE_PB: i16 {
    PLAYER_IDLE = 0,
    COUNT,
};

void Resources::create_sprites() {
    // Texture2D pj_texture = Resources::get_texture("PJ2_SS");

    // Sprites[PLAYER_IDLE] = s_Sprite(0, TILE_SIZE * 7, TILE_SIZE, TILE_SIZE, 0, 0, 0, 0, 11, 
    //     true, false, false, false, pj_texture);
    // Sprites[PLAYER_IDLE].id = PLAYER_IDLE;
}

// s_Sprite get_sprite(int id)
// {
//     auto s = Sprites[id];
//     s.id = id;
//     return s;
// };



// namespace Prefabs {
//     struct prefab {
//         s_Transform trans;
//         s_Sprite sprite;
//         s_Collider col;
//         s_Data data;
//     };
//
//     prefab PLAYER(fvec2 pos) {
//         s_Data d = {
//             d.Kind = s_Data::PLAYER
//         };
//
//         d.next_anim_id = Resources::PLAYER_RUNNING;
//         f32 offset = TILE_SIZE * 0.25f;
//         return prefab {
//             .trans = {pos, {0, 0}, {0, 0}},
//             .sprite = Resources::get_sprite(Resources::PLAYER_RUNNING),
//             .col = Rectangle{pos.x + offset, pos.y + offset - 2.0f, TILE_SIZE - offset * 2.0f, TILE_SIZE - offset},
//             .data = d,
//         };
//     }

    // prefab PIG(fvec2 pos) {
    //     s_Data d = {};
    //     d.Kind = s_Data::PIG;
    //
    //     d.next_anim_id = Resources::PIG_WALK;
    //
    //     return prefab {
    //         .trans = {pos, {0, 0}, {0, 0}},
    //         .sprite = Resources::get_sprite(Resources::PIG_WALK),
    //         .col = Rectangle{pos.x, pos.y, TILE_SIZE, TILE_SIZE},
    //         .data = d,
    //     };
    // }


void Resources::Init() {
    load_textures();
    load_fx();
    load_music();
    load_scenes();
    create_sprites();

};

