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
	}


	// call after all chunks are initialized
	void InitAllocator()
	{
		// allocate big buffer (1GB)
		// TODO: vary the allocation size based on some user setting
		allocator = std::make_unique<BufferAllocator>(1'000'000'000, 2 * sizeof(GLint));
		allocatorSplat = std::make_unique<BufferAllocator>(500'000'000, sizeof(GLint));


		
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
		auto cam = Renderer::GetPipeline()->GetCamera(0);
		std::atomic_int index = 0;
		DrawArraysIndirectCommand* comms = new DrawArraysIndirectCommand[allocator->ActiveAllocs()];

		int baseInstance = 0;

		const auto& allocs_ = allocator->GetAllocs();
		std::for_each(std::execution::par_unseq, // parallel and vectorization-safe
			allocs_.begin(), allocs_.end(), [&](const auto& alloc)
		{
			if (alloc.handle != NULL)
			{
				DrawArraysIndirectCommand cmd;
				cmd.count = (alloc.size / allocator->align_) - 2; // first two vertices are reserved
				cmd.instanceCount = 1;
				cmd.first = alloc.offset / allocator->align_;
				cmd.baseInstance = cmd.first; // same stride as vertices

				if (reinterpret_cast<Chunk*>(alloc.userdata)->IsVisible(*cam))
				{
					int i = index.fetch_add(1);
					comms[i] = cmd;
				}
			}
		});

		renderCount = index;
		dib = std::make_unique<DIB>(comms, index * sizeof(DrawArraysIndirectCommand));
		delete[] comms;
	}


	void GenerateDrawCommandsSplat()
	{
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
				DrawArraysIndirectCommand cmd;
				cmd.count = (alloc.size / allocatorSplat->align_) - 3; // first three vertices are reserved
				cmd.instanceCount = 1;
				cmd.first = alloc.offset / allocatorSplat->align_;
				cmd.baseInstance = cmd.first; // same stride as vertices

				if (reinterpret_cast<Chunk*>(alloc.userdata)->IsVisible(*cam))
				{
					int i = index.fetch_add(1);
					comms[i] = cmd;
				}
			}
		});

		renderCountSplat = index;
		dibSplat = std::make_unique<DIB>(comms, index * sizeof(DrawArraysIndirectCommand));
		delete[] comms;
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