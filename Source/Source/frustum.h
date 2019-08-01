#pragma once

struct Chunk;

class Frustum
{
public:
	enum Plane { Right, Left, Bottom, Top, Front, Back };
	enum { A, B, C, D };

	inline Frustum() {}
	inline virtual ~Frustum() {}

	void Transform(const glm::mat4& proj, const glm::mat4& view);

	// culling
	enum Visibility { Invisible, Partial, Full };
	Visibility IsInside(const glm::vec3& point) const;
	Visibility IsInside(const Chunk& box) const;

	inline glm::vec4 GetPlane(Plane plane) const
	{
		return glm::vec4(data_[plane][A], data_[plane][B], data_[plane][C], data_[plane][D]);
	}

private:
	void Normalize(Plane plane);
	float data_[6][4];
};