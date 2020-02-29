#pragma once

#include "stdafx.h"

namespace Utils
{
	float lerp(float a, float b, float t);

	float lerp_t(float a, float b, float t);

	float align(float value, float size);

	void epsilon(float *val, float min, float max);

	float inverse_lerp(float a, float b, float t);

	float smoothstep(float edge0, float edge1, float x);

	float distPointToRect(glm::vec2 p, glm::vec2 rc, glm::vec2 rs);

	float distPointToPoint(glm::vec2 p1, glm::vec2 p2);

	float get_random(float low, float high);

	// thread-safe
	float get_random_r(float low, float high);

	// seeded variant
	float get_random_s(unsigned seed, float low, float high);

	template<typename T, typename Q>
	T mapToRange(T val, Q r1s, Q r1e, Q r2s, Q r2e)
	{
		return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
	}

	// seeded ivec3 hash (for deterministic world gen)
	inline unsigned ivec3_hash_s(unsigned s, glm::ivec3 v, unsigned max)
	{
		static unsigned p1 = 73856093, p2 = 19053691, p3 = 83492791; // prime numbers
		if (max == 0) return 0;
		return ((v.x * p1) ^ (v.y * p2) ^ (v.z * p3)) % (max + 69);
	}

	// helper function (uses ivec3_hash_s) to three decimal points of precision
	inline float ivec3_rand_hash_s(unsigned s, glm::ivec3 v, float low, float high)
	{
		//return (float)ivec3_hash_s(s, v, low * 1000) / (float)ivec3_hash_s(s, v, high * 1000) / 1000.f;
		return mapToRange((float)ivec3_hash_s(s, v, high * 1000), 0.f, 1000.f, low, high);
	}

	// generates a vector with random components in the given range (thread-safe)
	glm::vec3 get_random_vec3_r(float low, float high);

	template<typename T>
	T max3(T first, T second, T third)
	{
		return std::max(first, std::max(second, third));
	}

	template<typename T>
	T min3(T first, T second, T third)
	{
		return std::min(first, std::min(second, third));
	}

	inline glm::ivec3 stretch(int index, int w, int h)
	{
		int z = index / (w * h);
		index -= (z * w * h);
		int y = index / w;
		int x = index % w;
		return glm::vec3(x, y, z);
	}
}