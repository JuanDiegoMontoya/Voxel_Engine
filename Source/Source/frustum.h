#pragma once

struct Chunk;

class Frustum
{
public:
	enum Plane { Right, Left, Bottom, Top, Front, Back };
	enum { A, B, C, D };

	Frustum() : data_() {}
	virtual ~Frustum() {}

	void Transform(const glm::mat4& proj, const glm::mat4& view);

	// culling
	enum class Visibility { Invisible, Partial, Full };
	Visibility IsInside(const glm::vec3& point) const;
	Visibility IsInside(const Chunk& box) const;

	glm::vec4 GetPlane(Plane plane) const
	{
		return glm::vec4(data_[int(plane)][A], data_[int(plane)][B], data_[int(plane)][C], data_[int(plane)][D]);
	}

private:
	void Normalize(Plane plane);
	float data_[6][4];
};