#include "stdafx.h"
#include "ChunkRenderer.h"
#include <dib.h>
#include <vao.h>
#include <vbo.h>
#include "BufferAllocator.h"
#include "chunk.h"
#include <camera.h>
#include <Pipeline.h>
#include "Renderer.h"
#include <execution>
#include "vendor/ctpl_stl.h"

namespace ChunkRenderer
{
	// Variables
	namespace
	{
		std::unique_ptr<VAO> vao;
		std::unique_ptr<VAO> vaoSplat;
		std::unique_ptr<DIB> dib;
		std::unique_ptr<DIB> dibSplat;
		int renderCount = 0;
		int renderCountSplat = 0;

		std::unique_ptr<ctpl::thread_pool> threads;
		int maxConcurrency;
		std::vector<std::future<void>> futures;
		std::atomic_int nextIdx;
	}

	void genCommands(int id, int start,
		DrawArraysIndirectCommand* cmds, int numThreads, Camera* cam) // atomic_int& no worky
	{
		int stride = numThreads;
		const auto& allocs_ = allocator->GetAllocs();
		int aaa = 0;
		for (int i = start; i < allocs_.size(); i += stride)
		{
			const auto& alloc = allocs_[i];
			if (alloc.handle != NULL)
			{
				Chunk* chunk = reinterpret_cast<Chunk*>(alloc.userdata);
				if (glm::distance(glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE), cam->GetPos()) > 800)
					continue;
				if (!chunk->IsVisible(*cam))
					continue;

				DrawArraysIndirectCommand cmd;
				cmd.count = (alloc.size / allocator->align_) - 2; // first 2 vertices are reserved
				cmd.instanceCount = 1;
				cmd.first = alloc.offset / allocator->align_;
				cmd.baseInstance = cmd.first; // same stride as vertices

				int idxx = nextIdx.fetch_add(1);
				cmds[idxx] = cmd;
			}
		}
	}


	// call after all chunks are initialized
	void InitAllocator()
	{
		maxConcurrency = glm::max(std::thread::hardware_concurrency(), 1u);
		threads = std::make_unique<ctpl::thread_pool>(maxConcurrency);

		// allocate big buffer (1GB)
		// TODO: vary the allocation size based on some user setting
		allocator = std::make_unique<BufferAllocator<void*>>(2'000'000'000, 2 * sizeof(GLint));
		allocatorSplat = std::make_unique<BufferAllocator<void*>>(200'000'000, sizeof(GLint));


		
		/* :::::::::::BUFFER FORMAT:::::::::::
		                        CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
		       | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
		First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
		Draw commands will specify where in memory the draw call starts. This will account for variable offsets.

		   :::::::::::BUFFER FORMAT:::::::::::*/


		vao = std::make_unique<VAO>();
		vao->Bind();
		// bind big data buffer (interleaved)
		glBindBuffer(GL_ARRAY_BUFFER, allocator->GetGPUHandle());
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1); // only 1 instance of a chunk should render, so divisor *should* be infinity
		GLuint offset = 0;
		// stride is sizeof(vertex) so baseinstance can be set to cmd.first and work (hopefully)
		glVertexAttribIPointer(2, 3, GL_INT, 2 * sizeof(GLuint), (void*)offset); // chunk position (one per instance)
		offset += sizeof(glm::ivec4); // move forward by TWO vertex sizes (vertex aligned)

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)offset); // encoded data
		offset += 1 * sizeof(GLfloat);

		glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 2 * sizeof(GLuint), (void*)offset); // lighting
		vao->Unbind();

		vaoSplat = std::make_unique<VAO>();
		vaoSplat->Bind();
		glBindBuffer(GL_ARRAY_BUFFER, allocatorSplat->GetGPUHandle());
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribDivisor(1, 1);
		glVertexAttribIPointer(1, 3, GL_INT, sizeof(GLuint), (void*)0);
		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), (void*)(3 * sizeof(GLint)));
		vaoSplat->Unbind();
	}


	void GenerateDrawCommands()
	{
		PERF_BENCHMARK_START;

		DrawArraysIndirectCommand* comms = new DrawArraysIndirectCommand[allocator->ActiveAllocs()];
		Camera* cam = Renderer::GetPipeline()->GetCamera(0);

		int threadNum = 12;
		for (int i = 0; i < threadNum; i++)
		{
			futures.push_back(threads->push(genCommands, i, comms, threadNum, cam));
		}



		// TODO: "finish" or "sync" function that does all this cleanup stuff
		// (also allows us to begin generating commands early and not have to immediately join
		for (auto& fut : futures)
		{
			fut.get();
		}

		renderCount = nextIdx;
		dib = std::make_unique<DIB>(comms, renderCount * sizeof(DrawArraysIndirectCommand));
		delete[] comms;
		nextIdx.store(0);
		futures.clear();

		PERF_BENCHMARK_END;
	}


	//void GenerateDrawCommands()
	//{
	//	PERF_BENCHMARK_START;
	//	DrawArraysIndirectCommand* comms = new DrawArraysIndirectCommand[allocator->ActiveAllocs()];
	//	Camera* cam = Renderer::GetPipeline()->GetCamera(0);
	//	std::atomic_int index = 0;

	//	const auto& allocs_ = allocator->GetAllocs();
	//	std::for_each(std::execution::par_unseq, // parallel and vectorization-safe
	//		allocs_.begin(), allocs_.end(), [&](const auto& alloc)
	//	{
	//		if (alloc.handle != NULL)
	//		{
	//			Chunk* chunk = reinterpret_cast<Chunk*>(alloc.userdata);
	//			if (glm::distance(glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE), cam->GetPos()) > 800)
	//				return;
	//			if (!chunk->IsVisible(*cam))
	//				return;

	//			DrawArraysIndirectCommand cmd;
	//			cmd.count = (alloc.size / allocator->align_) - 2; // first 2 vertices are reserved
	//			cmd.instanceCount = 1;
	//			cmd.first = alloc.offset / allocator->align_;
	//			cmd.baseInstance = cmd.first; // same stride as vertices

	//			int i = index.fetch_add(1);
	//			comms[i] = cmd;
	//		}
	//	});

	//	renderCount = index;
	//	dib = std::make_unique<DIB>(comms, renderCount * sizeof(DrawArraysIndirectCommand));
	//	delete[] comms;
	//	PERF_BENCHMARK_END;
	//}

	void GenerateDrawCommandsSplat()
	{
		PERF_BENCHMARK_START;
		auto cam = Renderer::GetPipeline()->GetCamera(0);
		std::atomic_int index = 0;
		DrawArraysIndirectCommand* comms = new DrawArraysIndirectCommand[allocatorSplat->ActiveAllocs()];

		int baseInstance = 0;

		const auto& allocs_ = allocatorSplat->GetAllocs();
		std::for_each(std::execution::par_unseq, // parallel and vectorization-safe
			allocs_.begin(), allocs_.end(), [&](const auto& alloc)
		{
			if (alloc.handle != NULL)
			{
				Chunk* chunk = reinterpret_cast<Chunk*>(alloc.userdata);
				if (glm::distance(glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE), cam->GetPos()) <= 800)
					return;
				if (!chunk->IsVisible(*cam))
					return;

				DrawArraysIndirectCommand cmd;
				cmd.count = (alloc.size / allocatorSplat->align_) - 3; // first three vertices are reserved
				cmd.instanceCount = 1;
				cmd.first = alloc.offset / allocatorSplat->align_;
				cmd.baseInstance = cmd.first; // same stride as vertices

				int i = index.fetch_add(1);
				comms[i] = cmd;
			}
		});

		renderCountSplat = index;
		dibSplat = std::make_unique<DIB>(comms, index * sizeof(DrawArraysIndirectCommand));
		delete[] comms;
		PERF_BENCHMARK_END;
	}


	void Render()
	{
		if (renderCount == 0)
			return;

		vao->Bind();
		dib->Bind();
		glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, renderCount, 0);
	}


	void RenderSplat()
	{
		if (renderCountSplat == 0)
			return;
	
		vaoSplat->Bind();
		dibSplat->Bind();
		glMultiDrawArraysIndirect(GL_POINTS, (void*)0, renderCountSplat, 0);
	}
}