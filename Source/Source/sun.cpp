#include "stdafx.h"
#include "sun.h"
#include "vao.h"
#include "vbo.h"
#include "vbo_layout.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
//#define LIl|IL yeet

Sun::Sun()
{
	vao_ = new VAO();
	vbo_ = new VBO(Render::square_vertices_3d, sizeof(Render::square_vertices_3d));
	VBOlayout layout;
	layout.Push<float>(3);
	vao_->AddBuffer(*vbo_, layout);
	vbo_->Unbind();
	vao_->Unbind();

	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO_);
	// create depth texture
	glGenTextures(1, &depthMapTex_);
	glBindTexture(GL_TEXTURE_2D, depthMapTex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowSize_.x, shadowSize_.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex_, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	near_ = 0.1f;
	far_ = 200.f;
}

void Sun::Update()
{
	dir_.x = cos(glfwGetTime());
	dir_.y = sin(glfwGetTime());
	dir_.z = -.2f;
	//dir_ = glm::vec3(.3f, -1, -0.5);
	pos_ = Render::GetCamera()->GetPos() - dir_ * 100.f;
	pos_ = -dir_ * 100.f;
	
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_, far_);
	glm::mat4 lightView = glm::lookAt(pos_, dir_, glm::vec3(0.0, 1.0, 0.0));
	sunViewProj_ = lightProjection * lightView;
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
	glClear(GL_DEPTH_BUFFER_BIT);
}