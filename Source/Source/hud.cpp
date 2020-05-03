#include "stdafx.h"
#include "hud.h"
#include "input.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "Renderer.h"

void HUD::Update()
{
	int ofs = Input::Mouse().scrollOffset.y;
	int num = int(selected_) + ofs;
	//if (num >= int(BlockType::bCount) || num < 0)
	//	ofs = 0;
	if (ofs)
	{
		//printf("%d\n", selected_);
		selected_ = (BlockType)glm::clamp(num, 0, (int)BlockType::bCount - 1);
		//printf("%d\n", selected_);
	}
	//reinterpret_cast<unsigned char&>(selected_) += ofs;
	Camera* cam = Renderer::GetPipeline()->GetCamera(0);
	//printf("%f\n", Input::Mouse().scrollOffset.y);

	// render the selected object on the screen
	ShaderPtr curr = Shader::shaders["flat_color"];
	curr->Use();
	glm::vec3 pos = glm::vec3(0, -6, -12);// +cam->GetPos() + cam->front * 10.f;
	glm::vec3 rot(.5, 1, 0);
	glm::vec3 scl(1, 1, 1);
	glm::mat4 model =
		glm::translate(glm::mat4(1), pos) *
		glm::rotate(glm::mat4(1), (float)glfwGetTime(), rot) *
		glm::scale(glm::mat4(1), scl);
	curr->setMat4("u_model", model);
	curr->setMat4("u_proj", cam->GetProj());
	curr->setMat4("u_view", glm::mat4(1));
	curr->setVec4("u_color", Block::PropertiesTable[int(selected_)].color);
	Renderer::DrawCube();
	curr->setVec4("u_color", glm::vec4(1));
	model = glm::scale(model, { 1.1, 1.1, 1.1 });
	curr->setMat4("u_model", model);


	GLint polygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	Renderer::DrawCube();
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
}
