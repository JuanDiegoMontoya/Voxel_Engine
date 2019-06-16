#pragma once

/**
* @file Frustum.h
* @brief
*/

#pragma once

#include "block.h"

namespace root
{
	class Frustum
	{
	public:
		enum Plane { Right, Left, Bottom, Top, Front, Back };
		enum { A, B, C, D };

		//! CTOR/DTOR:
		inline Frustum() {}
		inline virtual ~Frustum() {}

		//! SERVICES:
		void Transform(const glm::mat4& proj, const glm::mat4& view);
		void Normalize(Plane plane);

		//! CULLING:
		enum Visibility { Completely, Partially, Invisible };
		Visibility IsInside(const glm::vec3& point) const;
		Visibility IsInside(const Block& box) const;

		inline glm::vec4 GetPlane(Plane plane) const
		{
			return glm::vec4(_data[plane][A], _data[plane][B], _data[plane][C], _data[plane][D]);
		}

	private:
		float _data[6][4];
	};

	/*----------------------------------------------------------------------------*/

}