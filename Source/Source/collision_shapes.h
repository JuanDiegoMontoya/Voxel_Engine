#pragma once
#include "block.h"
#include "camera.h"

// abstract class
//struct Collider
//{
//	template<typename T>
//	bool CheckCollision(const T& other);
//};

// AABB
struct Box
{
	// 1x1x1 block
	Box(const glm::vec3& wpos)
	{
		min = wpos - .5f;
		max = wpos + .5f;
		min += .5f;
		max += .5f;
		blockpos = wpos;
	}

	// .5 x .5 x .5 camera
	Box(const Camera& c)
	{
		const auto& p = c.GetPos();
		min = p - .25f;
		max = p + .25f;
		//max.y *= 2;
		min.y -= 1.25f;
	}

	bool IsColliding(const Box& b) const
	{
		return	(this->min.x <= b.max.x && this->max.x >= b.min.x) &&
						(this->min.y <= b.max.y && this->max.y >= b.min.y) &&
						(this->min.z <= b.max.z && this->max.z >= b.min.z);
	}

	glm::vec3 GetPosition() const
	{
		return (max + min) / 2.f;
	}

	glm::vec3 GetScale() const
	{
		return (max - min) / 2.f;
	}

	//glm::vec3 size;
	//glm::vec3 position;
	glm::vec3 min;
	glm::vec3 max;
	glm::ivec3 blockpos;
};