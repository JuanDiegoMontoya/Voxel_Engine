#include "stdafx.h"
#include "NuRenderer.h"
#include "Renderer.h" // for old rendering functions
#include "World.h"
#include <Engine.h>
#include <shader.h>
#include <camera.h>

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
	}

	void Clear()
	{
		auto cc = World::bgColor_;
		glClearColor(cc.r, cc.g, cc.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void DrawAll()
	{
		PERF_BENCHMARK_START;

		Clear();
		glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

		Renderer::drawSky();
		drawChunks();
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
		//currShader->setVec3("viewPos", cam->GetPos());
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
		//currShader->setFloat("fogStart", loadD - loadD / 2.f);
		//currShader->setFloat("fogEnd", loadD - Chunk::CHUNK_SIZE * 1.44f); // cuberoot(3)
		//currShader->setVec3("fogColor", skyColor);

		std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
			[&](const std::pair<glm::ivec3, Chunk*>& pair)
		{
			ChunkPtr chunk = pair.second;
			if (chunk && chunk->IsVisible(*cam))
			{
				// set some uniforms, etc
				currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());
				currShader->setVec3("u_pos", Chunk::CHUNK_SIZE * chunk->GetPos());
				chunk->Render();
			}
		});
	}

	void drawChunksWater()
	{

	}
}