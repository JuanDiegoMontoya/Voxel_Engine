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

	template<typename T, typename Q>
	T mapToRange(T val, Q r1s, Q r1e, Q r2s, Q r2e)
	{
		return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
	}

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

	//DebugLineData PointsToASLine(const glm::vec2& p1, const glm::vec2& p2);
}