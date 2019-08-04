#include "stdafx.h"
#include "chunk.h"
#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "shader.h"
#include "directional_light.h"
#include "camera.h"
#include "pipeline.h"
#include "settings.h"
#include "sun.h"
#include "input.h"

#include "render.h"

// draws everything at once, I guess?
// this should be fine
void Renderer::DrawAll()
{
	glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

	drawShadows();
	drawSky();
	drawNormal();
	drawAxisIndicators();

	drawDepthMapsDebug();

	glDisable(GL_FRAMEBUFFER_SRGB);
}

void Renderer::Clear()
{
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::drawShadows()
{
	{
		//DrawCB preDrawCB =
		//	[this]()
		//{
		//	// 1. render depth of scene to texture (from light's perspective)
		//	//glDisable(GL_CULL_FACE);
		//	//glCullFace(GL_FRONT);

		//	ShaderPtr currShader = Shader::shaders["shadow"];
		//	currShader->Use();

		//	glViewport(0, 0, activeDirLight_->GetShadowSize().x, activeDirLight_->GetShadowSize().y);
		//	for (unsigned i = 0; i < activeDirLight_->GetNumCascades(); i++)
		//	{
		//		activeDirLight_->bindForWriting(i);
		//		glClear(GL_DEPTH_BUFFER_BIT);
		//		currShader->setMat4("lightSpaceMatrix", activeDirLight_->GetShadowOrthoProjMtxs()[i]);// *dirLight.GetView());
		//	}
		//};

		//ModelCB drawCB =
		//	[](const glm::mat4& model)
		//{
		//	Shader::shaders["shadow"]->setMat4("model", model);
		//};

		//DrawCB postDrawCB =
		//	[]()
		//{
		//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//	glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
		//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//};

		//drawChunks(false, preDrawCB, drawCB, postDrawCB);
	}

	// 1. render depth of scene to texture (from light's perspective)
	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glViewport(0, 0, activeDirLight_->GetShadowSize().x, activeDirLight_->GetShadowSize().y);



	glm::mat4 view = Render::GetCamera()->GetView();
	glm::mat4 vView[3];
	glm::vec3 LitDir = glm::normalize(activeDirLight_->GetPos());
	glm::vec3 right = glm::normalize(glm::cross(LitDir, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 up = glm::normalize(glm::cross(right, LitDir));
	glm::mat4 LitViewFam = glm::lookAt(activeDirLight_->GetPos(), Render::GetCamera()->GetPos(), up);

	activeDirLight_->calcOrthoProjs(LitViewFam);
	for (unsigned int i = 0; i < 3; ++i)
	{
		vView[i] = glm::lookAt(activeDirLight_->GetModlCent(i), activeDirLight_->GetModlCent(i) + LitDir * .2f, up);
	}



	ShaderPtr currShader = Shader::shaders["shadow"];
	currShader->Use();

	for (unsigned i = 0; i < activeDirLight_->GetNumCascades(); i++)
	{
		activeDirLight_->bindForWriting(i);
		glClear(GL_DEPTH_BUFFER_BIT);

		//currShader->setMat4("lightSpaceMatrix", activeDirLight_->GetShadowOrthoProjMtxs()[i]);
		currShader->setMat4("lightSpaceMatrix", activeDirLight_->GetProjMat(vView[i], i) * vView[i]);

		// draw objects in world
		std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
			[&](std::pair<glm::ivec3, Chunk*> chunk)
		{
			if (chunk.second)// && chunk.second->IsVisible())
			{
				currShader->setMat4("model", chunk.second->GetModel());
				chunk.second->Render();
			}
		});
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawSky()
{
	activeSun_->Render();
}

void Renderer::drawNormal()
{
	DrawCB preDrawCB =
		[this]()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // don't forget to reset original culling face
		

		glm::mat4 vView[3];
		glm::vec3 LitDir = glm::normalize(activeDirLight_->GetPos());
		glm::vec3 right = glm::normalize(glm::cross(LitDir, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, LitDir));
		for (unsigned int i = 0; i < 3; ++i)
		{
			vView[i] = glm::lookAt(activeDirLight_->GetModlCent(i), activeDirLight_->GetModlCent(i) + LitDir * .2f, up);
		}
		glm::mat4 projection = Render::GetCamera()->GetProj();
		glm::vec4 cascadEnds = activeDirLight_->GetCascadeEnds();
		glm::vec3 cascadeEndsClipSpace =
			glm::vec3((projection*glm::vec4(0.0f, 0.0f, -cascadEnds[1], 1.0f)).z,
			(projection*glm::vec4(0.0f, 0.0f, -cascadEnds[2], 1.0f)).z,
				(projection*glm::vec4(0.0f, 0.0f, -cascadEnds[3], 1.0f)).z);
		glm::vec3 ratios(
			activeDirLight_->GetRatio(vView[0], 0),
			activeDirLight_->GetRatio(vView[1], 1),
			activeDirLight_->GetRatio(vView[2], 2)
		);


		// render blocks in each active chunk
		ShaderPtr currShader = Shader::shaders["chunk_shaded"];
		currShader->Use();
		currShader->setMat4("u_view", Render::GetCamera()->GetView());
		currShader->setMat4("u_proj", Render::GetCamera()->GetProj());
		currShader->setVec3("viewPos", Render::GetCamera()->GetPos());
		currShader->setVec3("lightPos", activeDirLight_->GetPos());
		//currShader->setVec3("ratios", ratios);
		//currShader->setMat4("lightSpaceMatrix", dirLight.GetViewProj());
		
		std::vector<float> zVals;
		for (int i = 0; i < activeDirLight_->GetNumCascades(); i++)
		{
			glm::vec4 vView(0, 0, activeDirLight_->GetCascadeEnds()[i + 1], 1);
			glm::vec4 vClip = Render::GetCamera()->GetProj() * vView;
			zVals.push_back(vClip.z);
		}

		glm::mat4 liteMats[3];
		liteMats[0] = activeDirLight_->GetProjMat(vView[0], 0) * vView[0];
		liteMats[1] = activeDirLight_->GetProjMat(vView[1], 1) * vView[1];
		liteMats[2] = activeDirLight_->GetProjMat(vView[2], 2) * vView[2];
		glUniformMatrix4fv(currShader->Uniforms["lightSpaceMatrix"],
			3, GL_FALSE,
			&liteMats[0][0][0]);
		glUniform1fv(currShader->Uniforms["cascadeEndClipSpace"], 3, &cascadeEndsClipSpace[0]);
		//glUniform1fv(currShader->Uniforms["cascadeEndClipSpace"], 3, &zVals[0]);

		//currShader->set1FloatArray("cascadeEndClipSpace", zVals, zVals.size());
		//glUniformMatrix4fv(currShader->Uniforms["lightSpaceMatrix"], 
		//	activeDirLight_->GetNumCascades(), GL_FALSE, 
		//	&activeDirLight_->GetShadowOrthoProjMtxs()[0][0][0]);
		//currShader->setMat4("lightSpaceMatrix[0]", activeDirLight_->GetShadowOrthoProjMtxs()[0]);
		//currShader->setVec3("dirLight.direction", dirLight.GetDir());
		currShader->setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
		currShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		currShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		activeDirLight_->bindForReading();
	};

	ModelCB drawCB =
		[](const glm::mat4& model)
	{
		Shader::shaders["chunk_shaded"]->setMat4("u_model", model);
	};

	DrawCB postDrawCB = [](){}; // does nothing (yet)

	drawChunks(true, preDrawCB, drawCB, postDrawCB);
}

void Renderer::drawPostProcessing()
{
}

void Renderer::drawChunks(
	bool cullFrustum,
	DrawCB predraw_cb, 
	ModelCB draw_cb, 
	DrawCB postdraw_cb)
{
	predraw_cb();

	if (cullFrustum)
	{
		std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
			[&](std::pair<glm::ivec3, Chunk*> chunk)
		{
			if (chunk.second && chunk.second->IsVisible())
			{
				draw_cb(chunk.second->GetModel());
				chunk.second->Render();
			}
		});
	}
	else
	{
		std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
			[&](std::pair<glm::ivec3, Chunk*> chunk)
		{
			if (chunk.second)// && chunk.second->IsVisible())
			{
				draw_cb(chunk.second->GetModel());
				chunk.second->Render();
			}
		});
	}

	postdraw_cb();
}

void Renderer::drawBillboard(VAO * vao, size_t count, DrawCB uniform_cb)
{
	// TODO: set this up for actual use
}

void Renderer::drawDepthMapsDebug()
{
	// debug shadows
	if (Input::Keyboard().down[GLFW_KEY_4])
	{
		// TODO: figure out how to do this without making everything draw to last framebuffer
		Shader::shaders["debug_shadow"]->Use();
		for (int i = 0; i < 3; i++)
		{
			glViewport(0 + 512 * i, 0, 512, 512);
			Shader::shaders["debug_shadow"]->setInt("depthMap", i);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, activeDirLight_->GetDepthTex()[i]);
			drawQuad();
		}
	}
}

void Renderer::drawAxisIndicators()
{
	static VAO* axisVAO;
	static VBO* axisVBO;
	if (axisVAO == nullptr)
	{
		float indicatorVertices[] =
		{
			// positions			// colors
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};

		axisVAO = new VAO();
		axisVBO = new VBO(indicatorVertices, sizeof(indicatorVertices), GL_STATIC_DRAW);
		VBOlayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		axisVAO->AddBuffer(*axisVBO, layout);
	}
	/* Renders the axis indicator (a screen-space object) as though it were
		one that exists in the world for simplicity. */
	ShaderPtr currShader = Shader::shaders["axis"];
	currShader->Use();
	Camera* cam = Render::GetCamera();
	currShader->setMat4("u_model", glm::translate(glm::mat4(1), cam->GetPos() + cam->front * 10.f)); // add scaling factor (larger # = smaller visual)
	currShader->setMat4("u_view", cam->GetView());
	currShader->setMat4("u_proj", cam->GetProj());
	glClear(GL_DEPTH_BUFFER_BIT); // allows indicator to always be rendered
	axisVAO->Bind();
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, 6);
	axisVAO->Unbind();
}

// draw the shadow map/view of the world from the sun's perspective
void Renderer::drawQuad()
{
	static unsigned int quadVAO = 0;
	static unsigned int quadVBO;
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}