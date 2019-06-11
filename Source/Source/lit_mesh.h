#pragma once
#include "mesh.h"

class Lit_Mesh : public Mesh
{
public:
	void Draw(ShaderPtr shader) override;
	using Mesh::Mesh;
};