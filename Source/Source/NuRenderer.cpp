#include "stdafx.h"
#include "NuRenderer.h"
#include "Renderer.h" // for old rendering functions
#include "World.h"
#include <Engine.h>
#include <shader.h>
#include <camera.h>
#include <input.h>
#include "ChunkStorage.h"
#include <dib.h>
#include "ChunkRenderer.h"

namespace NuRenderer
{
	void Init()
	{
		CompileShaders();
		Engine::PushRenderCallback(DrawAll, 0);
	}


	void CompileShaders()
	{
		Shader::shaders["chunk_optimized"] = new Shader("chunk_optimized.vs", "chunk_optimized.fs");
		Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
		Shader::shaders["compact_batch"] = new Shader("compact_batch.cs");
	}


	void Clear()
	{
		drawCalls = 0;
		auto cc = World::bgColor_;
		glClearColor(cc.r, cc.g, cc.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}


	void DrawAll()
	{
		PERF_BENCHMARK_START;

		Clear();
		glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction
		glEnable(GL_PROGRAM_POINT_SIZE);

		if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
		{
			if (Input::Keyboard().down[GLFW_KEY_1])
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (Input::Keyboard().down[GLFW_KEY_2])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			if (Input::Keyboard().down[GLFW_KEY_3])
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}

		Renderer::drawSky();
		drawChunks();
		splatChunks();
		//drawChunksMultiIndirect();
		drawChunksWater();
		Renderer::drawAxisIndicators();
		//Renderer::postProcess();

		glDisable(GL_FRAMEBUFFER_SRGB);
		PERF_BENCHMARK_END;
	}


	void drawChunks()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // don't forget to reset original culling face

		// render blocks in each active chunk
		ShaderPtr currShader = Shader::shaders["chunk_optimized"];
		currShader->Use();

		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		float angle = glm::max(glm::dot(-glm::normalize(Renderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
		currShader->setFloat("sunAngle", angle);
		//printf("Angle: %f\n", angle);
		// currShader->setInt("textureAtlas", ...);

		float loadD = World::chunkManager_.GetLoadDistance();
		float loadL = World::chunkManager_.GetUnloadLeniency();
		// undo gamma correction for sky color
		static glm::vec3 skyColor(
			glm::pow(.529f, 2.2f),
			glm::pow(.808f, 2.2f),
			glm::pow(.922f, 2.2f));
		currShader->setVec3("viewPos", cam->GetPos());
		currShader->setFloat("fogStart", loadD - loadD / 2.f);
		currShader->setFloat("fogEnd", loadD - Chunk::CHUNK_SIZE * 1.44f); // cuberoot(3)
		currShader->setVec3("fogColor", skyColor);
		currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());




		//ChunkRenderer::GenerateDrawCommands();
		ChunkRenderer::GenerateDrawCommandsGPU();
		ChunkRenderer::Render();
		return;






		auto& chunks = ChunkStorage::GetMapRaw();
		auto it = chunks.begin();
		auto end = chunks.end();
		for (int i = 0; it != end; ++it)
		{
			ChunkPtr chunk = it->second;
			if (chunk 
				&& chunk->IsVisible(*cam) 
				//&& glm::distance(cam->GetPos(), glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE)) < 200
				)
			{
				// set some uniforms, etc
				//currShader->setVec3("u_pos", Chunk::CHUNK_SIZE * chunk->GetPos());
				chunk->Render();
				drawCalls++;
			}
		}
	}


	void splatChunks()
	{
		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		auto proj = cam->GetProj();
		auto view = cam->GetView();
		ShaderPtr currShader = Shader::shaders["chunk_splat"];
		currShader->Use();

		currShader->setVec3("u_viewpos", cam->GetPos());

		currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());
		currShader->setMat4("u_invProj", glm::inverse(proj));
		currShader->setMat4("u_invView", glm::inverse(view));

		float angle = glm::max(glm::dot(-glm::normalize(Renderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
		currShader->setFloat("sunAngle", angle);

		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		currShader->setVec2("u_viewportSize", glm::vec2(vp[2], vp[3]));





		ChunkRenderer::GenerateDrawCommandsSplat();
		ChunkRenderer::RenderSplat();
		return;






		std::for_each(ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end(),
			[&](const std::pair<glm::ivec3, Chunk*>& pair)
		{
			ChunkPtr chunk = pair.second;
			if (chunk && chunk->IsVisible(*cam) &&
				glm::distance(cam->GetPos(), glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE)) >= 200)
			{
				// set some uniforms, etc
				currShader->setVec3("u_pos", Chunk::CHUNK_SIZE * chunk->GetPos());
				chunk->RenderSplat();
				drawCalls++;
			}
		});
	}


	void drawChunksWater()
	{

	}

	GLuint gIndirectBuffer = 0;
	std::vector<DrawElementsIndirectCommand> drawCommands;
	void generateDrawCommands()
	{
		ASSERT(0);
		drawCommands.clear();

		GLuint baseVert = 0;
		
		auto& chunks = ChunkStorage::GetMapRaw();
		auto it = chunks.begin();
		auto end = chunks.end();
		for (int i = 0; it != end; ++it)
		{
			Chunk* chunk = it->second;
			//DrawElementsIndirectCommand cmd = chunk->GetMesh().GetDrawCommand(baseVert, i);
			//drawCommands.push_back(cmd);
		}

		//feed the draw command data to the gpu
		glGenBuffers(1, &gIndirectBuffer);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
		glBufferData(GL_DRAW_INDIRECT_BUFFER, drawCommands.size() * sizeof(DrawElementsIndirectCommand), drawCommands.data(), GL_DYNAMIC_DRAW);

		//feed the instance id to the shader. (not needed in this case)
		//glBindBuffer(GL_ARRAY_BUFFER, gIndirectBuffer);
		//glEnableVertexAttribArray(2);
		//glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(SDrawElementsCommand), (void*)(offsetof(DrawElementsIndirectCommand, baseInstance)));
		//glVertexAttribDivisor(2, 1); //only once per instance
	}


	void drawChunksMultiIndirect()
	{
		glMultiDrawElementsIndirect(
			GL_TRIANGLES,
			GL_UNSIGNED_INT,
			(void*)0,
			drawCommands.size(),
			0); // draw commands are tightly packed
	}
}