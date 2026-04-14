#pragma once

#ifndef VMATH
#define VMATH

#include "vtypes.h"



fvec2 fVec2Subs(fvec2 v1, fvec2 v2);
fvec2 fVec2Add(fvec2 v1, fvec2 v2);
fvec2 fVec2AddScalar(fvec2 v, f32 s);
fvec2 fVec2xScalar(fvec2 v, f32 s);
vec3 Vec3xScalar(vec3 v, f32 s);
f32 Signf32(f32 val);
fvec2 fVec2Normalize(fvec2 v);
vec3 Vec3Normalize(vec3 v);
vec3 Vec3Cross(vec3 v1, vec3 v2);
f32 Clampf32(f32 v, f32 min, f32 max);
f32 fDot(fvec2 v1, fvec2 v2);
f32 fAngle2Vectors(fvec2 v1, fvec2 v2);
f32 Lerp(f32 start, f32 end, f32 amount);
f32 EaseOutCubic(f32 number);
f32 Maxf32(f32 a, f32 b);
f32 Minf32(f32 a, f32 b);
f32 Absf32(f32 val);
f32 fVec2Length(fvec2 v);
f32 Vec3Length(vec3 v);
fvec2 fVec2Unit(fvec2 v);


#endif


