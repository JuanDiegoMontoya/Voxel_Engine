#include "stdafx.h"
#include "unlit_mesh.h"

void Unlit_Mesh::Draw(ShaderPtr shader)
{
	shader->Use();

	shader->setMat4("u_model", model);
	shader->setMat4("u_view", view);
	shader->setMat4("u_proj", proj);

	// since we don't have an ibo with 36 positions
	if (indexed)
		renderer.Draw(*vao, *ibo, *shader);
	else
		renderer.DrawArrays(*vao, vertices.size(), *shader);
	vao->Unbind();
}
