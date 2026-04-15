#include "vmath.h"
#include "math.h"
#include "vtypes.h"

constexpr auto PI = 3.14159265359f;


f32 fPow(f32 val, f32 power)
{
    f32 ret = 1.0f;
    while (power-->0){
        ret *= val;
    }
    return ret;
}


f32 Signf32(f32 val)
{
    if (val == 0)
        return 0;

    return val > 0.0f ? 1.0f: -1.0f;
}

fvec2 fVec2xScalar(fvec2 v, f32 s)
{
    return {v.x * s, v.y * s};
}
vec3 Vec3xScalar(vec3 v, f32 s)
{
    return {v.x * s, v.y * s, v.z * s};
}
fvec2 fVec2Add(fvec2 v1, fvec2 v2)
{
    return {v1.x+v2.x, v1.y+v2.y};
}

fvec2 fVec2AddScalar(fvec2 v, f32 s)
{
    return {v.x+s, v.y+s};
}

fvec2 fVec2Subs(fvec2 v1, fvec2 v2)
{
    return {v1.x-v2.x, v1.y-v2.y};
}

f32 Minf32(f32 a, f32 b)
{
    return a < b ? a : b;
}

f32 Maxf32(f32 a, f32 b)
{
    return a > b ? a : b;
}

vec3 Vec3Cross(vec3 v1, vec3 v2)
{
    return {
        (v1.y * v2.z) - (v1.z * v2.y),
        (v1.z * v2.x) - (v1.x * v2.z),
        (v1.x * v2.y) - (v1.y * v2.x),
    };
}

fvec2 fVec2Normalize(fvec2 v)
{
    float mag = fVec2Length(v); 
    return mag == 0 ? fvec2{0, 0} : fvec2{v.x /mag, v.y/mag};
}

vec3 Vec3Normalize(vec3 v)
{
    float mag = Vec3Length(v); 
    return mag == 0 ? vec3{0, 0, 0} : vec3{v.x /mag, v.y/mag, v.z/mag};
}


f32 Clampf32(f32 v, f32 min, f32 max)
{
    return Minf32(Maxf32(v, min), max);
}

f32 Absf32(f32 val)
{
    return val > 0 ? val : -val;
}

fvec2 fVec2Unit(fvec2 v)
{
    f32 mag = fVec2Length(v);
    return mag == 0 ? fvec2{0, 0} : fvec2{v.x /mag, v.y/mag};
}

f32 EaseOutCubic(f32 number)
{
    return f32(sin((number * PI) / 2.0f));}


f32 Lerp(f32 start, f32 end, f32 amount)
{
    return start + amount*(end-start);
}

f32 Dot(fvec2 v1, fvec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

f32 fVec2Length(fvec2 v)
{
    return sqrtf(powf(v.x, 2) + powf(v.y, 2)); 
}

f32 Vec3Length(vec3 v)
{
    return sqrtf(powf(v.x, 2) + powf(v.y, 2) + powf(v.z, 2)); 
}


f32 fAngle2Vectors(fvec2 v1, fvec2 v2)
{
    f32 dot = Dot(v1, v2);
    f32 mag1 = fVec2Length(v1);
    f32 mag2 = fVec2Length(v2);


    f32 cos = dot / (mag1 * mag2);

    if (cos >  1) cos =  1;
    if (cos < -1) cos = -1;
    
    return acosf(cos);
}

