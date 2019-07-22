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
	frustum_ = new Frustum();
	vao_ = new VAO();
	vbo_ = new VBO(Render::square_vertices_3d, sizeof(Render::square_vertices_3d));
	VBOlayout layout;
	layout.Push<float>(3);
	vao_->AddBuffer(*vbo_, layout);
	vbo_->Unbind();
	vao_->Unbind();
	
	initCascadedShadowMapFBO();


	near_ = 0.1f;
	far_ = 130.f;
	dir_ = glm::vec3(.3f, -1, -0.5);
	pos_ = -dir_ * 200.f;
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
	
	glm::mat4 lightProjection = glm::ortho(-projSize, projSize, -projSize, projSize, near_, far_);
	//glm::mat4 lightView = glm::lookAt(pos_, dir_, glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 lightView = glm::lookAt(pos_, Render::GetCamera()->GetPos(), glm::vec3(0.0, 1.0, 0.0));
	sunViewProj_ = lightProjection * lightView;
	view_ = lightView;
	//frustum_->Transform(lightProjection, lightView);

	// equally spaced cascade ends (may change in future)
	cascadeEnds_[0] = near_;
	cascadeEnds_[1] = far_ / 3.f;
	cascadeEnds_[2] = far_ / 3.f * 2;
	cascadeEnds_[3] = far_;
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
	//glClear(GL_DEPTH_BUFFER_BIT);
}

void Sun::initCascadedShadowMapFBO()
{
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO_);
	// create depth texture
	glGenTextures(shadowCascades_, &depthMapTexes_[0]);

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	for (unsigned i = 0; i < shadowCascades_; i++)
	{
		glBindTexture(GL_TEXTURE_2D, depthMapTexes_[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowSize_.x, shadowSize_.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexes_[0], 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
		"Shadow cascade framebuffer incomplete.");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Sun::bindForWriting(unsigned index)
{
	ASSERT(index < shadowCascades_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthMapFBO_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexes_[index], 0);
}

void Sun::bindForReading()
{
	switch (shadowCascades_) // fall-through intended
	{
	case 3:
		glActiveTexture(2);
		glBindTexture(GL_TEXTURE_2D, depthMapTexes_[2]);
	case 2:
		glActiveTexture(1);
		glBindTexture(GL_TEXTURE_2D, depthMapTexes_[1]);
	case 1:
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexes_[0]);
	}
}

void Sun::calcOrthoProjs()
{
	// Get the inverse of the view transform
	glm::mat4 Cam = Render::GetCamera()->GetView();
	glm::mat4 CamInv = glm::inverse(Cam);

	// Get the light space tranform
	glm::mat4 LightM = view_;;

	float ar = (float)Settings::Graphics.screenX / Settings::Graphics.screenY;
	float fov = Render::GetCamera()->GetFov(); // degrees
	float tanHalfHFOV = tanf(glm::radians(fov / 2.0f));
	float tanHalfVFOV = tanf(glm::radians((fov * ar) / 2.0f));

	for (unsigned i = 0; i < shadowCascades_; i++)
	{
		float xn = cascadeEnds_[i] * tanHalfHFOV;
		float xf = cascadeEnds_[i + 1] * tanHalfHFOV;
		float yn = cascadeEnds_[i] * tanHalfVFOV;
		float yf = cascadeEnds_[i + 1] * tanHalfVFOV;

		glm::vec4 frustumCorners[8] =
		{
			// near face
			glm::vec4(xn, yn, cascadeEnds_[i], 1.0),
			glm::vec4(-xn, yn, cascadeEnds_[i], 1.0),
			glm::vec4(xn, -yn, cascadeEnds_[i], 1.0),
			glm::vec4(-xn, -yn, cascadeEnds_[i], 1.0),

			// far face
			glm::vec4(xf, yf, cascadeEnds_[i + 1], 1.0),
			glm::vec4(-xf, yf, cascadeEnds_[i + 1], 1.0),
			glm::vec4(xf, -yf, cascadeEnds_[i + 1], 1.0),
			glm::vec4(-xf, -yf, cascadeEnds_[i + 1], 1.0)
		};
		glm::vec4 frustumCornersL[8];

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();

		for (unsigned j = 0; j < 8; j++)
		{

			// Transform the frustum coordinate from view to world space
			glm::vec4 vW = CamInv * frustumCorners[j];

			// Transform the frustum coordinate from world to light space
			frustumCornersL[j] = LightM * vW;

			minX = glm::min(minX, frustumCornersL[j].x);
			maxX = glm::max(maxX, frustumCornersL[j].x);
			minY = glm::min(minY, frustumCornersL[j].y);
			maxY = glm::max(maxY, frustumCornersL[j].y);
			minZ = glm::min(minZ, frustumCornersL[j].z);
			maxZ = glm::max(maxZ, frustumCornersL[j].z);
		}

		shadowOrthoProjInfo_[i].r = maxX;
		shadowOrthoProjInfo_[i].l = minX;
		shadowOrthoProjInfo_[i].b = minY;
		shadowOrthoProjInfo_[i].t = maxY;
		shadowOrthoProjInfo_[i].f = maxZ;
		shadowOrthoProjInfo_[i].n = minZ;
		shadowOrthoProjMtxs_[i] = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	}
}
