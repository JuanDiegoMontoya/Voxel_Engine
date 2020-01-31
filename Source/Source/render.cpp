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
#include "chunk_manager.h"

#include "render.h"

#define VISUALIZE_MAPS 0

Renderer::Renderer()
 : activeDirLight_(), activeSun_(), 
	gBuffer(), gPosition(), gNormal(), gAlbedoSpec(), gDepth(), rboDepth(),
	pBuffer(), pColor(), pDepth(),
	view(), projection(), vView(), LitDir(), right(), up(),
	LitViewFam(), ratios(), cascadEnds(), cascadeEndsClipSpace() {}

// initializes the gBuffer and its attached textures
void Renderer::Init()
{
	initDeferredBuffers();
	initPPBuffers();

	GLint count;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);

	for (GLint i = 0; i < count; ++i)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		if (!strcmp(extension, "GL_NVX_gpu_memory_info"))
			nvUsageEnabled = true;
			//printf("%d: %s\n", i, extension);
	}
}

// draws everything at once, I guess?
// this should be fine
void Renderer::DrawAll()
{
  PERF_BENCHMARK_START;
	glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

	//std::cout << "RENDER" << std::endl;

	//glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);
	if (doGeometryPass)
		geometryPass();
	
	if (renderShadows)
		drawShadows();
	drawSky();
	drawNormal();
	drawWater();
	drawAxisIndicators();
	drawDepthMapsDebug();

	//glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);

	postProcess();
	//drawPostProcessing();

	glDisable(GL_FRAMEBUFFER_SRGB);
  PERF_BENCHMARK_END;
}

void Renderer::Clear()
{
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::ClearCSM()
{
	for (int i = 0; i < activeDirLight_->GetNumCascades(); i++)
	{
		activeDirLight_->bindForWriting(i);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearTexSubImage(
			activeDirLight_->GetDepthTex()[i], 0, 0, 0, 0,
			activeDirLight_->GetShadowSize().x, activeDirLight_->GetShadowSize().y, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::ClearGGuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// OpenGL 4.4+
	int scrX = Settings::Graphics.screenX;
	int scrY = Settings::Graphics.screenY;
	glClearTexSubImage(gPosition, 0, 0, 0, 0, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
	glClearTexSubImage(gNormal, 0, 0, 0, 0, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
	glClearTexSubImage(gAlbedoSpec, 0, 0, 0, 0, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glClearTexSubImage(gDepth, 0, 0, 0, 0, scrX, scrY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

static VAO* blockHoverVao = nullptr;
static VBO* blockHoverVbo = nullptr;
void Renderer::DrawCube()
{
	if (blockHoverVao == nullptr)
	{
		blockHoverVao = new VAO();
		blockHoverVbo = new VBO(Render::cube_vertices, sizeof(Render::cube_vertices));
		VBOlayout layout;
		layout.Push<float>(3);
		blockHoverVao->AddBuffer(*blockHoverVbo, layout);
	}
	//glClear(GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_CULL_FACE);
	blockHoverVao->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_CULL_FACE);
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

	//https://github.com/niley1nov/cascaded-exponential-shadow-mapping/blob/master/src/main8.cpp#L363
	// CSM stuff
	view = Render::GetCamera()->GetView();
	LitDir = glm::normalize(-activeDirLight_->GetPos());
	right = glm::normalize(glm::cross(LitDir, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, LitDir));
	LitViewFam = glm::lookAt(activeDirLight_->GetPos(), Render::GetCamera()->GetPos(), up);
	cascadEnds = activeDirLight_->GetCascadeEnds();
	projection = Render::GetCamera()->GetProj();
	cascadeEndsClipSpace =
		glm::vec3((projection*glm::vec4(0.0f, 0.0f, -cascadEnds[1], 1.0f)).z,
		(projection*glm::vec4(0.0f, 0.0f, -cascadEnds[2], 1.0f)).z,
			(projection*glm::vec4(0.0f, 0.0f, -cascadEnds[3], 1.0f)).z);
	ratios = glm::vec3(
		activeDirLight_->GetRatio(vView[0], 0),
		activeDirLight_->GetRatio(vView[1], 1),
		activeDirLight_->GetRatio(vView[2], 2)
	);
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
				//chunk.second->RenderWater();
			}
		});
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// draw sky objects (then clear the depth buffer)
void Renderer::drawSky()
{
	activeSun_->Render();
}

// draw solid objects in each chunk
void Renderer::drawNormal()
{
	DrawCB preDrawCB =
		[this]()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // don't forget to reset original culling face

		// render blocks in each active chunk
		ShaderPtr currShader = Shader::shaders["chunk_shaded"];
		currShader->Use();
		currShader->setMat4("u_view", Render::GetCamera()->GetView());
		currShader->setMat4("u_proj", Render::GetCamera()->GetProj());
		currShader->setVec3("viewPos", Render::GetCamera()->GetPos());
		currShader->setVec3("lightPos", activeDirLight_->GetPos());
		//currShader->setVec3("ratios", ratios);
		float loadD = chunkManager_->GetLoadDistance();
		float loadL = chunkManager_->GetUnloadLeniency();
		// undo gamma correction
		static glm::vec3 skyColor(
			glm::pow(.529f, 2.2f),
			glm::pow(.808f, 2.2f),
			glm::pow(.922f, 2.2f));
		currShader->setFloat("fogStart", loadD - loadD / 2.f);
		currShader->setFloat("fogEnd", loadD - Chunk::CHUNK_SIZE * 1.44f); // cuberoot(3)
		currShader->setVec3("fogColor", skyColor);

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
		currShader->setVec3("dirLight.direction", activeSun_->GetDir());
		currShader->setBool("computeShadow", renderShadows);
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

// draw water in each chunk
void Renderer::drawWater()
{
	glDisable(GL_CULL_FACE);
	//glDisable(GL_BLEND); // TODO: only skip blending between other water blocks (prolly just use another framebuffer)
	
	ShaderPtr currShader = Shader::shaders["chunk_water"];
	currShader->Use();
	std::vector<int> values = { 0, 1, 2 };
	currShader->setIntArray("shadowMap", values, values.size());
	currShader->setInt("ssr_positions", 3);
	//currShader->setInt("ssr_normals", 4);
	currShader->setInt("ssr_albedoSpec", 5);
	currShader->setInt("ssr_depth", 6);

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

	currShader->setFloat("u_time", (float)glfwGetTime());
	currShader->setMat4("u_view", Render::GetCamera()->GetView());
	currShader->setMat4("u_proj", Render::GetCamera()->GetProj());
	currShader->setMat4("inv_projection", glm::inverse(Render::GetCamera()->GetProj()));
	currShader->setVec3("viewPos", Render::GetCamera()->GetPos());
	currShader->setVec3("lightPos", activeDirLight_->GetPos());
	currShader->setBool("computeSSR", doGeometryPass);
	currShader->setBool("computeShadow", renderShadows);

	// fog
	float loadD = chunkManager_->GetLoadDistance();
	float loadL = chunkManager_->GetUnloadLeniency();
	// undo gamma correction
	static glm::vec3 skyColor(
		glm::pow(.529f, 2.2f),
		glm::pow(.808f, 2.2f),
		glm::pow(.922f, 2.2f));
	currShader->setFloat("fogStart", loadD - loadD / 2.f);
	currShader->setFloat("fogEnd", loadD - Chunk::CHUNK_SIZE * 1.44f); // cuberoot(3)
	currShader->setVec3("fogColor", skyColor);

	//currShader->setVec3("ssr_skyColor", glm::vec3(.529f, .808f, .922f));

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

	currShader->setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
	currShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	currShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
	currShader->setVec3("dirLight.direction", activeSun_->GetDir());
	activeDirLight_->bindForReading();
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	currShader->setInt("ssr_positions", 3);
	//currShader->setInt("ssr_normals", 4);
	currShader->setInt("ssr_albedoSpec", 5);

	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)// && chunk.second->IsVisible())
		{
			currShader->setMat4("u_model", chunk.second->GetModel());
			chunk.second->RenderWater();
		}
	});
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

// all post processing effects
void Renderer::drawPostProcessing()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind default framebuffer
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int scrX = Settings::Graphics.screenX;
	int scrY = Settings::Graphics.screenY;

	ShaderPtr shader = Shader::shaders["postprocess"];
	shader->Use();
	shader->setInt("colorMap", 0);
	shader->setInt("depthMap", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pColor);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pDepth);
	//shader->setFloat("near_plane", Render::GetCamera()->GetNear());
	//shader->setFloat("far_plane", Render::GetCamera()->GetFar());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);
	drawQuad();

	glEnable(GL_DEPTH_TEST);
}


//void Renderer::drawDebug()
//{
//	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
//		[&](std::pair<glm::ivec3, Chunk*> chunk)
//		{
//			if (chunk.second && (cullFrustum ? chunk.second->IsVisible() : true))
//			{
//				auto model = chunk.second->GetModel();
//				glDisable(GL_CULL_FACE);
//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//				model = glm::translate(model, { 1,1,1 });
//				draw_cb(glm::scale(model, { 32, 32, 32 }));
//				DrawCube();
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				glEnable(GL_CULL_FACE);
//			}
//		});
//}


void Renderer::drawChunks(
	bool cullFrustum,
	DrawCB predraw_cb, 
	ModelCB draw_cb, 
	DrawCB postdraw_cb)
{
	predraw_cb();

	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second && (cullFrustum ? chunk.second->IsVisible() : true))
		{
			auto model = chunk.second->GetModel();
			draw_cb(model);
			chunk.second->Render();
		}
	});

	postdraw_cb();
}

void Renderer::drawBillboard(VAO * vao, size_t count, DrawCB uniform_cb)
{
	// TODO: set this up for actual use
}

void Renderer::drawDepthMapsDebug()
{
	Shader::shaders["debug_shadow"]->Use();
	Shader::shaders["debug_shadow"]->setFloat("near_plane", Render::GetCamera()->GetNear());
	Shader::shaders["debug_shadow"]->setFloat("far_plane", Render::GetCamera()->GetFar());

	// debug shadows
	if (Input::Keyboard().down[GLFW_KEY_4])
	{
		Shader::shaders["debug_shadow"]->Use();
		Shader::shaders["debug_shadow"]->setInt("perspective", 0);
		for (int i = 0; i < 3; i++)
		{
			glViewport(0 + 512 * i, 0, 512, 512);
			Shader::shaders["debug_shadow"]->setInt("depthMap", i);
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, activeDirLight_->GetDepthTex()[i]);
			drawQuad();
		}
	}

	if (Input::Keyboard().down[GLFW_KEY_5])
	{
		Shader::shaders["debug_map3"]->Use();
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

		Shader::shaders["debug_map3"]->setInt("map", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		drawQuad();
	}

	if (Input::Keyboard().down[GLFW_KEY_8])
	{
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
		Shader::shaders["debug_shadow"]->Use();
		Shader::shaders["debug_shadow"]->setInt("depthMap", 3);
		Shader::shaders["debug_shadow"]->setInt("perspective", 1);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gDepth);

		drawQuad();
	}

	if (Input::Keyboard().down[GLFW_KEY_9])
	{
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
		Shader::shaders["debug_map3"]->Use();
		Shader::shaders["debug_map3"]->setInt("map", 4);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, pColor);

		drawQuad();
	}

	if (Input::Keyboard().down[GLFW_KEY_0])
	{
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
		Shader::shaders["debug_shadow"]->Use();
		Shader::shaders["debug_shadow"]->setInt("depthMap", 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, pDepth);

		drawQuad();
	}

#if VISUALIZE_MAPS
	if (Input::Keyboard().down[GLFW_KEY_6])
	{
		Shader::shaders["debug_map3"]->Use();
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

		Shader::shaders["debug_map3"]->setInt("map", 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		drawQuad();
	}

	if (Input::Keyboard().down[GLFW_KEY_7])
	{
		Shader::shaders["debug_map3"]->Use();
		glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

		Shader::shaders["debug_map3"]->setInt("map", 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		drawQuad();
	}

#endif
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

void Renderer::initDeferredBuffers()
{
	int scrX = Settings::Graphics.screenX;
	int scrY = Settings::Graphics.screenY;

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// 'color' indicates that the data is stored in a color format
	// position 'color' buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
#if VISUALIZE_MAPS
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal 'color' buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
#if VISUALIZE_MAPS
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	
	// color + specular 'color' buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// depth attachment
	glGenTextures(1, &gDepth);
	glBindTexture(GL_TEXTURE_2D, gDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, scrX, scrY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

	unsigned int attachments[4] = { 
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, 
		GL_COLOR_ATTACHMENT2, GL_DEPTH_ATTACHMENT };
	glDrawBuffers(3, attachments);
	
	// attach depth renderbuffer
	//glGenRenderbuffers(1, &rboDepth);
	//glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scrX, scrY);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// render deferred geometry and fully opaque surfaces (aka not-water)
void Renderer::geometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ShaderPtr currShader = Shader::shaders["chunk_geometry"];
	currShader->Use();
	currShader->setMat4("projection", Render::GetCamera()->GetProj());
	currShader->setMat4("view", Render::GetCamera()->GetView());

	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[&](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)// && chunk.second->IsVisible())
		{
			currShader->setMat4("model", chunk.second->GetModel());
			chunk.second->Render();
			//chunk.second->RenderWater();
		}
	});
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// lite up dat mf'in geometry we got there
void Renderer::lightingPass()
{
}

void Renderer::initPPBuffers()
{
	int scrX = Settings::Graphics.screenX;
	int scrY = Settings::Graphics.screenY;

	glGenFramebuffers(1, &pBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);

	// color buffer
	glGenTextures(1, &pColor);
	glBindTexture(GL_TEXTURE_2D, pColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pColor, 0);

	// depth attachment
	glGenTextures(1, &pDepth);
	glBindTexture(GL_TEXTURE_2D, pDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, scrX, scrY, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pDepth, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::postProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int scrX = Settings::Graphics.screenX;
	int scrY = Settings::Graphics.screenY;

	// copy default framebuffer contents into pBuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pBuffer);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(
		0, 0, scrX, scrY, 
		0, 0, scrX, scrY, 
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	//glBlitFramebuffer(
	//	0, 0, scrX / 2, scrY / 2,
	//	0, 0, scrX / 2, scrY / 2,
	//	GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pColor);
	ShaderPtr shader = Shader::shaders["postprocess"];
	shader->Use();

	shader->setInt("colorTex", 0);
	shader->setBool("sharpenFilter", ppSharpenFilter);
	shader->setBool("edgeDetection", ppEdgeDetection);
	shader->setBool("chromaticAberration", ppChromaticAberration);
	shader->setBool("blurFilter", ppBlurFilter);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);
	drawQuad();
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// copy pBuffer contents into default framebuffer
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, pBuffer);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//glBlitFramebuffer(
	//	0, 0, scrX / 2, scrY / 2,
	//	0, 0, scrX / 2, scrY / 2,
	//	GL_COLOR_BUFFER_BIT, GL_LINEAR);
	//glBlitFramebuffer(
	//	0, 0, scrX / 2, scrY / 2,
	//	0, 0, scrX / 2, scrY / 2,
	//	GL_DEPTH_BUFFER, GL_NEAREST); // must be GL_NEAREST for depth
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glEnable(GL_DEPTH_TEST);
}

// draws a single quad over the entire viewport
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