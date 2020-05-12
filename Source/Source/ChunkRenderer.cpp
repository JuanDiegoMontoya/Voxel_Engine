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
		
		//std::unique_ptr<VBO> posVbo;
	}


	// call after all chunks are initialized
	void InitAllocator()
	{
		allocator = std::make_unique<ChunkVBOAllocator>(1'000'000'000);

		vao = std::make_unique<VAO>();

		vao->Bind();
		
		/* :::::::::::BUFFER FORMAT:::::::::::
		                 CHUNK 1                                 CHUNK 2
		| cpos, encoded+lighting, encoded+lighting, ... | cpos, encoded+lighting, ... |
		
		Draw commands will specify where in memory the draw call starts. This will account for variable offsets.

		   :::::::::::BUFFER FORMAT:::::::::::*/


		// bind big data buffer (interleaved)
		allocator->GetVBO()->Bind();
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribDivisor(2, 1); // only 1 instance of a chunk should render, so divisor *should* be infinity
		GLuint offset = 0;
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)offset); // chunk position (one per instance)
		offset += 3 * sizeof(glm::vec3);

		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)offset); // encoded data
		offset += 1 * sizeof(GLfloat);

		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)offset); // lighting
	}

	void GenerateDrawCommands()
	{
		std::vector<DrawArraysIndirectCommand> commands = allocator->GetCommands();
	}

	void Render()
	{
		if (commands.size() == 0)
			return;

		glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)0, commands.size(), 0);
	}
}