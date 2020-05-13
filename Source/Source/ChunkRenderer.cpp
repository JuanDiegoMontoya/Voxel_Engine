#include "stdafx.h"
#include "ChunkRenderer.h"
#include <dib.h>
#include <vao.h>
#include <vbo.h>
#include "ChunkVBOAllocator.h"

namespace ChunkRenderer
{
	// Variables
	namespace
	{
		std::vector<DrawArraysIndirectCommand> commands;
		std::unique_ptr<VAO> vao;
		//std::unique_ptr<VBO> posvbo;
		std::unique_ptr<DIB> dib;
	}


	// call after all chunks are initialized
	void InitAllocator()
	{
		// allocate big buffer (1GB)
		// TODO: vary the allocation size based on some user setting
		allocator = std::make_unique<ChunkVBOAllocator>(2'000'000'000, 2 * sizeof(GLint));

		vao = std::make_unique<VAO>();

		vao->Bind();
		
		/* :::::::::::BUFFER FORMAT:::::::::::
		                        CHUNK 1                                    CHUNK 2                   NULL                   CHUNK 3
		       | cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... | null (any length) | cpos, encoded+lighting, ... |
		First:   offset(CHUNK 1)=0                               offset(CHUNK 2)                                   offset(CHUNK 3)
		Draw commands will specify where in memory the draw call starts. This will account for variable offsets.

		   :::::::::::BUFFER FORMAT:::::::::::*/


		// bind big data buffer (interleaved)
		allocator->GetVBO()->Bind();
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
	}

	void GenerateDrawCommands()
	{
		commands = allocator->GetCommands();
		dib = std::make_unique<DIB>(commands.data(), commands.size() * sizeof(DrawArraysIndirectCommand));
	}

	void Render()
	{
		if (commands.size() == 0)
			return;

		vao->Bind();
		dib->Bind();
		glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, commands.size(), 0);
	}
}