#include "vstd/vtypes.h"
#include <memory>

struct Texture;
struct s_Sprite 
{
    i16 src_x;
    i16 src_y;
    i16 width;
    i16 height;
    i8 offset_x = 0;
    i8 offset_y = 0;
    i8 padding_x = 0;
    i8 padding_y = 0;
    i8 total_frames;
    i8 current_frame = 0;
    bool inverted = false;
    bool looping;
    bool lock;
    bool kill_on_finish;
	bool event_driven;
    i32 id;
    std::shared_ptr<Texture> texture;

    s_Sprite(
        i16 src_x, i16 src_y,
        i16 width, i16 height, i8 offset_x, i8 offset_y, i8 padding_x, i8 padding_y, 
        i8 total_frames, bool looping, bool lock, bool kill_on_finish, bool event_driven,
        std::shared_ptr<Texture> texture
    ):
        src_x(src_x), src_y(src_y), width(width), height(height), offset_x(offset_x), offset_y(offset_y), padding_x(padding_x),
        padding_y(padding_y), total_frames(total_frames), looping(looping), lock(lock), kill_on_finish(kill_on_finish),
        event_driven(event_driven), texture(texture){};
    s_Sprite() = default;

};

