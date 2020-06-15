#include "stdafx.h"
#include "ChunkRenderer.h"
#include <dib.h>
#include <vao.h>
#include <vbo.h>
#include "BufferAllocator.h"
#include "chunk.h"
#include <camera.h>
#include <Frustum.h>
#include <Pipeline.h>
#include "Renderer.h"
#include <execution>
#include "vendor/ctpl_stl.h"
#include <shader.h>
#include <abo.h>
#include <param_bo.h>
#include "ChunkStorage.h"

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

		//std::unique_ptr<ABO> drawCounter;
		//std::unique_ptr<ABO> drawCounterSplat;
		std::unique_ptr<Param_BO> drawCountGPU;
		std::unique_ptr<Param_BO> drawCountGPUSplat;

		const int blockSize = 64; // defined in compact_batch.cs

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
				//Chunk* chunk = reinterpret_cast<Chunk*>(alloc.userdata);
				//if (glm::distance(glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE), cam->GetPos()) > 800)
				//	continue;
				//if (!chunk->IsVisible(*cam))
				//	continue;
				AABB16 box = alloc.userdata;
				glm::vec3 cpos = (box.min + box.max) / 2.f;
				if (glm::distance(cpos, cam->GetPos()) > 800)
					continue;
				if (cam->GetFrustum()->IsInside(box) == Frustum::Visibility::Invisible)
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

		//drawCounter = std::make_unique<ABO>(1); // one atomic uint
		//drawCounter->Reset();
		drawCountGPU = std::make_unique<Param_BO>();
		//drawCounterSplat = std::make_unique<ABO>(1); // one atomic uint
		//drawCounterSplat->Reset();
		drawCountGPUSplat = std::make_unique<Param_BO>();

		// allocate big buffer
		// TODO: vary the allocation size based on some user setting
		allocator = std::make_unique<BufferAllocator<AABB16>>(2'000'000'000, 2 * sizeof(GLint));
		//allocator = std::make_unique<BufferAllocator<AABB16>>(100'000'000, 2 * sizeof(GLint));
		allocatorSplat = std::make_unique<BufferAllocator<AABB16>>(200'000'000, sizeof(GLint));

		
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

	void GenerateDrawCommandsGPU()
	{
		PERF_BENCHMARK_START;
#ifdef TRACY_ENABLE
		TracyGpuZone("Gen draw commands norm");
#endif

		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		// make buffer sized as if every allocation was non-null
		ShaderPtr sdr = Shader::shaders["compact_batch"];
		sdr->Use();
#if 1
		sdr->setVec3("u_viewpos", cam->GetPos());
		Frustum fr = *cam->GetFrustum();
		for (int i = 0; i < 5; i++) // ignore near plane
		{
			std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "]";
			sdr->set1FloatArray(uname.c_str(), fr.GetData()[i], 4);
		}
		sdr->setFloat("u_cullMinDist", settings.normalMin);
		sdr->setFloat("u_cullMaxDist", settings.normalMax);
#endif
		sdr->setUInt("u_reservedVertices", 2);
		sdr->setUInt("u_vertexSize", sizeof(GLuint) * 2);

		//drawCounter->Bind(0);
		//drawCounter->Reset();
		drawCountGPU->Reset();

		// copy input data to buffer at binding 0
		GLuint indata;
		glGenBuffers(1, &indata);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, indata);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, indata);
		const auto& allocs = allocator->GetAllocs();
		glBufferData(GL_SHADER_STORAGE_BUFFER, allocator->AllocSize() * allocs.size(), allocs.data(), GL_STATIC_COPY);

		// make DIB output SSBO (binding 1) for the shader
		dib = std::make_unique<DIB>(
			nullptr, 
			allocator->ActiveAllocs() * sizeof(DrawArraysIndirectCommand),
			GL_STATIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, dib->GetID());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dib->GetID());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCountGPU->GetID());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawCountGPU->GetID());
		
		{
			int numBlocks = (allocs.size() + blockSize - 1) / blockSize;
			glDispatchCompute(numBlocks, 1, 1);
			//glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		}
		//renderCount = drawCounter->Get(0); // sync point
		//ASSERT(renderCount <= allocator->ActiveAllocs());
		glDeleteBuffers(1, &indata);

		drawCountGPU->Unbind();

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
				//Chunk* chunk = reinterpret_cast<Chunk*>(alloc.userdata);
				//if (glm::distance(glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE), cam->GetPos()) <= 800)
				//	return;
				//if (!chunk->IsVisible(*cam))
				//	return;
				AABB box = alloc.userdata;
				glm::vec3 cpos = (box.min + box.max) / 2.f;
				if (glm::distance(cpos, cam->GetPos()) <= 800)
					return;
				if (cam->GetFrustum()->IsInside(box) == Frustum::Visibility::Invisible)
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


	void GenerateDrawCommandsSplatGPU()
	{
		PERF_BENCHMARK_START;
#ifdef TRACY_ENABLE
		TracyGpuZone("Gen draw commands splat");
#endif

		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		// make buffer sized as if every allocation was non-null
		ShaderPtr sdr = Shader::shaders["compact_batch"];
		sdr->Use();
#if 1
		sdr->setVec3("u_viewpos", cam->GetPos());
		Frustum fr = *cam->GetFrustum();
		for (int i = 0; i < 5; i++) // ignore near plane
		{
			std::string uname = "u_viewfrustum.data_[" + std::to_string(i) + "]";
			sdr->set1FloatArray(uname.c_str(), fr.GetData()[i], 4);
		}
		sdr->setFloat("u_cullMinDist", settings.splatMin);
		sdr->setFloat("u_cullMaxDist", settings.splatMax);
#endif
		sdr->setUInt("u_reservedVertices", 3);
		sdr->setUInt("u_vertexSize", sizeof(GLuint) * 1);

		//drawCounterSplat->Bind(0);
		//drawCounterSplat->Reset();
		drawCountGPUSplat->Reset();

		// copy input data to buffer at binding 0
		GLuint indata;
		glGenBuffers(1, &indata);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, indata);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, indata);
		const auto& allocs = allocatorSplat->GetAllocs();
		glBufferData(GL_SHADER_STORAGE_BUFFER, allocatorSplat->AllocSize() * allocs.size(), allocs.data(), GL_STATIC_COPY);

		// make DIB output SSBO (binding 1) for the shader
		dibSplat = std::make_unique<DIB>(
			nullptr,
			allocatorSplat->ActiveAllocs() * sizeof(DrawArraysIndirectCommand),
			GL_STATIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, dibSplat->GetID());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dibSplat->GetID());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawCountGPUSplat->GetID());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, drawCountGPUSplat->GetID());

		{
			int numBlocks = (allocs.size() + blockSize - 1) / blockSize;
			glDispatchCompute(numBlocks, 1, 1);
			//glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		}
		//renderCountSplat = drawCounterSplat->Get(0); // sync point
		//ASSERT(renderCountSplat <= allocatorSplat->ActiveAllocs());
		glDeleteBuffers(1, &indata);

		drawCountGPUSplat->Unbind();

		PERF_BENCHMARK_END;
	}


	void Render()
	{
#ifdef TRACY_ENABLE
		TracyGpuZone("Render chunks normal");
#endif
		//if (renderCount == 0)
		//	return;

		vao->Bind();
		dib->Bind();
		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		//glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, renderCount, 0);
		drawCountGPU->Bind();
		glMultiDrawArraysIndirectCount(GL_TRIANGLES, (void*)0, (GLintptr)0, allocator->ActiveAllocs(), 0);
	}


	void RenderSplat()
	{
#ifdef TRACY_ENABLE
		TracyGpuZone("Render chunks splat");
#endif
		//if (renderCountSplat == 0)
		//	return;
	
		vaoSplat->Bind();
		dibSplat->Bind();
		drawCountGPUSplat->Bind();
		//glMultiDrawArraysIndirect(GL_POINTS, (void*)0, renderCountSplat, 0);
		glMultiDrawArraysIndirectCount(GL_POINTS, (void*)0, (GLintptr)0, allocatorSplat->ActiveAllocs(), 0);
	}


	void DrawBuffers()
	{
		glDisable(GL_DEPTH_TEST);

		ShaderPtr sdr = Shader::shaders["buffer_vis"];
		sdr->Use();
		glm::mat4 model(1);
		model = glm::scale(model, { 1, 1, 1 });
		model = glm::translate(model, { -.5, -.90, 0 });
		sdr->setMat4("u_model", model);

		glLineWidth(50);
		allocator->Draw();

		glEnable(GL_DEPTH_TEST);
	}
}