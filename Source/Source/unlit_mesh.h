#pragma once
#include "mesh.h"

class Unlit_Mesh : public Mesh
{
public:
	void Draw(ShaderPtr shader) override;
	using Mesh::Mesh;

	glm::mat4 model, view, proj;
};