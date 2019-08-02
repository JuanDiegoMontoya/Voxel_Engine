#include "stdafx.h"
#include "frustum.h"
#include "sun.h"
#include "vao.h"
#include "vbo.h"
#include "vbo_layout.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "settings.h"
#include <limits>

Sun::Sun()
{
	vao_ = new VAO();
	vbo_ = new VBO(Render::square_vertices_3d, sizeof(Render::square_vertices_3d));
	VBOlayout layout;
	layout.Push<float>(3);
	vao_->AddBuffer(*vbo_, layout);
	vbo_->Unbind();
	vao_->Unbind();

	dir_ = glm::vec3(.3f, -1, -0.5);
	pos_ = -dir_ * 100.f;
}

void Sun::Update()
{
	if (orbits)
	{
		dir_.x = (float)cos(glfwGetTime());
		dir_.y = (float)sin(glfwGetTime());
		dir_.z = -.2f;
		pos_ = -dir_ * followDist + orbitPos;
	}

	if (followCam)
		pos_ = Render::GetCamera()->GetPos() - dir_ * followDist;
	
	//glm::mat4 lightView = glm::lookAt(pos_, dir_, glm::vec3(0.0, 1.0, 0.0));
	view_ = glm::lookAt(pos_, glm::normalize(Render::GetCamera()->GetPos()), glm::vec3(0.0, 1.0, 0.0));

	dirLight_.Update(pos_, dir_);
}

void Sun::Render()
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	vao_->Bind();
	vbo_->Bind();
	const glm::mat4& view = Render::GetCamera()->GetView();
	const glm::mat4& proj = Render::GetCamera()->GetProj();

	ShaderPtr currShader = Shader::shaders["sun"];
	currShader->Use();
	currShader->setMat4("VP", proj * view);
	currShader->setVec3("CameraRight", view[0][0], view[1][0], view[2][0]);
	currShader->setVec3("CameraUp", view[0][1], view[1][1], view[2][1]);
	currShader->setVec3("BillboardPos", pos_);
	currShader->setVec2("BillboardSize", 5.5f, 5.5f);

	currShader->setVec4("u_color", 1.f, 1.f, 0.f, 1.f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/* Clear the depth buffer after rendering the sun to simulate
			the sun being at infinity.
			Note: this means the sun should be rendered first each frame
			to prevent depth-related artifacts from appearing. */
	//glClear(GL_DEPTH_BUFFER_BIT); // uncomment to make sun appear to be in the sky
}