#include "stdafx.h"
#include "mesh.h"
#include "lit_mesh.h"
#include "shader.h"

void Lit_Mesh::Draw(ShaderPtr shader)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		std::string name = textures[i].GetType();
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);

		shader->setFloat(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].GetID());
	}
	glActiveTexture(GL_TEXTURE0);
	
	// draw mesh
	//glBindVertexArray(VAO);
	vao->Bind();
	//if (indexed)
	//	renderer.Draw(*vao, *ibo, *shader);
	//else
	//	renderer.DrawArrays(*vao, vertices.size(), *shader);
	vao->Unbind();
}