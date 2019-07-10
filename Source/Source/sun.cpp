#include "stdafx.h"
#include "sun.h"
#include "vao.h"
#include "vbo.h"
#include "vbo_layout.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"

Sun::Sun()
{
	vao_ = new VAO();
	vbo_ = new VBO(Render::square_vertices_3d, sizeof(Render::square_vertices_3d));
	VBOlayout layout;
	layout.Push<float>(3);
	vao_->AddBuffer(*vbo_, layout);
}

void Sun::Render()
{
	vao_->Bind();
	vbo_->Bind();
	const glm::mat4& view = Render::GetCamera()->GetView();
	const glm::mat4& proj = Render::GetCamera()->GetProj();

	// TODO: make sun move + actually be drawn
	ShaderPtr currShader = Shader::shaders["sun"];
	currShader->Use();
	currShader->setMat4("VP", proj * view);
	currShader->setVec3("CameraRight", view[0][0], view[1][0], view[2][0]);
	currShader->setVec3("CameraUp", view[0][1], view[1][1], view[2][1]);
	currShader->setVec3("BillboardPos", Render::GetCamera()->GetPos() - dir_ * 10.f);
	currShader->setVec2("BillboardSize", 1.f, 1.f);

	currShader->setVec4("u_color", 1.f, 1.f, 0.f, 1.f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/* Clear the depth buffer after rendering the sun to simulate
			the sun being at infinity.
			Note: this means the sun should be rendered first each frame
			to prevent depth-related artifacts from appearing. */
	glClear(GL_DEPTH_BUFFER_BIT);
}
