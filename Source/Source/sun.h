#pragma once
#include "vao.h"
#include "vbo.h"
#include "vbo_layout.h"
#include "pipeline.h"
#include "shader.h"

class Sun
{
public:
	Sun()
	{
		vao_ = new VAO();
		vbo_ = new VBO(Render::square_vertices, sizeof(Render::square_vertices));
		VBOlayout layout;
		layout.Push<float>(2);
		vao_->AddBuffer(*vbo_, layout);
	}

	void Update()
	{
		dir_.x = cos(glfwGetTime());
		dir_.y = sin(glfwGetTime());
		dir_.z = 0;
	}

	void Render()
	{
		vao_->Bind();
		vbo_->Bind();
		// TODO: make sun move + actually be drawn
		ShaderPtr currShader = Shader::shaders["sun"];
		currShader->Use();
		glDrawArrays(GL_TRIANGLES, 0, 2);
	}

	inline const glm::vec3& getDir() { return dir_; }

private:
	glm::vec3 dir_;

	VAO* vao_;
	VBO* vbo_;
};