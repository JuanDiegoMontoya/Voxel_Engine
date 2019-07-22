#include "stdafx.h"

#include <cmath>
#include <random>
#include "utilities.h"

namespace Utils
{
	float lerp(float a, float b, float t)
	{
		return (a + ((b - a) * t));
	}

	// 1 - t is the AMOUNT COMPLETED after one second (lower is faster)
	float lerp_t(float a, float b, float t)
	{
		float dt = 0; //= ...
		return (a + ((b - a) * (1 - pow(t, dt))));
	}

	float align(float value, float size)
	{
		//return std::floor(value / size) * size;

		//round is way better than floor
		return std::round(value / size) * size;
	}

	//sets a value to zero if it falls within a given range
	void epsilon(float *val, float min, float max)
	{
		if (*val > min && *val < max)
			*val = 0.0f;
	}

	float inverse_lerp(float a, float b, float t)
	{
		return ((t - a) / (b - a));
	}

	float smoothstep(float edge0, float edge1, float x)
	{
		x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return x * x * (3 - 2 * x);
	}

	float distPointToRect(glm::vec2 p, glm::vec2 rc, glm::vec2 rs)
	{
		float dx = glm::max(glm::abs(p.x - rc.x) - rs.x / 2.0f, 0.0f);
		float dy = glm::max(glm::abs(p.y - rc.y) - rs.y / 2.0f, 0.0f);

		return dx * dx + dy * dy;
	}

	float distPointToPoint(glm::vec2 p1, glm::vec2 p2)
	{
		float dx = glm::max(glm::abs(p1.x - p2.x), 0.0f);
		float dy = glm::max(glm::abs(p1.y - p2.y), 0.0f);

		return dx * dx + dy * dy;
	}

	float get_random(float low, float high)
	{
		// super fast and super random
		static std::random_device rd;
		static std::mt19937 rng(rd());
		std::uniform_real_distribution<float> dist(low, high);
		return (float)dist(rng);
	}

	float get_random_r(float low, float high)
	{
		static thread_local std::mt19937 generator;
		std::uniform_real_distribution<float> distribution(low, high);
		return distribution(generator);
	}

	float get_random_s(unsigned seed, float low, float high)
	{
		static std::random_device rd;
		static std::mt19937 rng(rd());
		rng.seed(seed);
		std::uniform_real_distribution<float> dist(low, high);
		return (float)dist(rng);
	}

	glm::vec3 get_random_vec3_r(float low, float high)
	{
		static thread_local std::mt19937 generator;
		std::uniform_real_distribution<float> distribution1(low, high);
		std::uniform_real_distribution<float> distribution2(low, high);
		std::uniform_real_distribution<float> distribution3(low, high);
		return glm::vec3(distribution1(generator), distribution2(generator), distribution3(generator));
	}


	// disregards color and width
	//DebugLineData PointsToASLine(const glm::vec2& p1, const glm::vec2& p2)
	//{
	//	glm::vec2 line;
	//	float angle;
	//	float magnitude;

	//	// uses distance formula to get the magnitude of the line
	//	float x = (p2.x - p1.x);
	//	float y = (p2.y - p1.y);
	//	float xSqr = x * x;
	//	float ySqr = y * y;
	//	float C = std::sqrt(xSqr + ySqr);

	//	line.x = (p1.x + p2.x) / 2.f;
	//	line.y = (p1.y + p2.y) / 2.f;

	//	angle = glm::atan(y, x);

	//	magnitude = C;

	//	return DebugLineData(glm::vec2(line.x, line.y), angle, magnitude, glm::vec3(1));
	//}
}