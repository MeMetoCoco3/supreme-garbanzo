
#pragma once
#ifndef VSTD
#define VSTD
#include <cstdio>
#include <string>
#include <stdint.h>

typedef uint32_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t   u8;

typedef int32_t b32;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t   i8;

typedef float f32;
typedef double f64;


constexpr unsigned int BIT(unsigned int x) { return 1u << x; }

struct vec2
{
    i32 x;
    i32 y;
	vec2() : x(0), y(0) {};
	vec2(i32 a, i32 b) : x(a), y(b) {}
	std::string to_string(void) const
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%d, %d\n", x, y);
		return buffer;
	}

	vec2 operator+(const vec2& other) const
	{
		return { x + other.x, y + other.y };
	}

	vec2& operator+=(const vec2& other) 
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	vec2 operator-(const vec2& other) const
	{
		return { x - other.x, y - other.y };
	}

	vec2& operator-=(const vec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
};



struct fvec2
{
    f32 x;
    f32 y;
	fvec2() : x(0.0f), y(0.0f) {};
	fvec2(f32 a, f32 b) : x(a), y(b) {}
	std::string to_string(void) const
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%.2f, %.2f", x, y);
		return buffer;
	}

	fvec2 operator+(const fvec2& other) const
	{
		return { x + other.x, y + other.y };
	}

	fvec2& operator+=(const fvec2& other) 
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	fvec2 operator-(const fvec2& other) const
	{
		return { x - other.x, y - other.y };
	}

	fvec2& operator-=(const fvec2& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
};

struct vec4
{
    f32 x;
    f32 y;
    f32 z;
    f32 w;

	vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
	vec4(f32 a, f32 b, f32 c, f32 d) : x(a), y(b), z(c), w(d){}

	std::string to_string(void) const
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%.2f, %.2f, %.2f, %.2f", x, y, z, w);
		return buffer;
	}
};



struct vec3
{
    f32 x;
    f32 y;
    f32 z;

	vec3() : x(0.0f), y(0.0f), z(0.0f) {};
	vec3(f32 a, f32 b, f32 c) : x(a), y(b), z(c) {}

	std::string to_string(void) const
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%.2f, %.2f, %.2f\n", x, y, z);
		return buffer;
	}

	vec3 operator+(const vec3& other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	vec3& operator+=(const vec3& other) 
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	vec3 operator-(const vec3& other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	vec3& operator-=(const vec3& other) 
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
};


#endif
