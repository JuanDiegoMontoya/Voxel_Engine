#include "stdafx.h"
#include "ChunkMesh.h"
#include <vao.h>
#include <vbo.h>
#include <ibo.h>
#include <iomanip>
#include "chunk.h"
#include "ChunkHelpers.h"
#include "ChunkStorage.h"

void ChunkMesh::BuildMesh(ChunkPtr curChunk)
{
	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	for (int i = 0; i < fCount; i++)
	{
		nearChunks[i] = ChunkStorage::GetChunk(
			curChunk->GetPos() + ChunkHelpers::faces[i]);
	}


	mtx.lock();

	glm::ivec3 pos;
	for (pos.z = 0; pos.z < Chunk::CHUNK_SIZE; pos.z++)
	{
		// precompute first flat index part
		int zcsq = pos.z * Chunk::CHUNK_SIZE_SQRED;
		for (pos.y = 0; pos.y < Chunk::CHUNK_SIZE; pos.y++)
		{
			// precompute second flat index part
			int yczcsq = pos.y * Chunk::CHUNK_SIZE + zcsq;
			for (pos.x = 0; pos.x < Chunk::CHUNK_SIZE; pos.x++)
			{
				// this is what we would be doing every innermost iteration
				//int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQRED;
				// we only need to do addition
				int index = pos.x + yczcsq;

				// skip fully transparent blocks
				const Block block = curChunk->BlockAt(index);
				if (Block::PropertiesTable[block.GetTypei()].invisible)
					continue;

				for (int f = Far; f < fCount; f++)
					buildBlockFace(f, pos, block);
				//buildBlockVertices_normal({ x, y, z }, block);
			}
		}
	}

	mtx.unlock();

	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	double milliseconds = benchmark_duration_.count() * 1000;
	if (accumcount > 1000)
	{
		accumcount = 0;
		accumtime = 0;
	}
	accumtime = accumtime + milliseconds;
	accumcount = accumcount + 1;
	//std::cout 
	//	<< std::setw(-2) << std::showpoint << std::setprecision(4) << accumtime / accumcount << " ms "
	//	<< "(" << milliseconds << ")"
	//	<< std::endl;
}

void ChunkMesh::BuildBuffers()
{
	mtx.lock();

	if (!vao_)
		vao_ = std::make_unique<VAO>();

	ibo_ = std::make_unique<IBO>(&tIndices[0], tIndices.size());

	vao_->Bind();
	encodedStuffVbo_ = std::make_unique<VBO>(encodedStuffArr.data(), sizeof(GLfloat) * encodedStuffArr.size());
	encodedStuffVbo_->Bind();
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)0); // encoded stuff
	glEnableVertexAttribArray(0);

	lightingVbo_ = std::make_unique<VBO>(lightingArr.data(), sizeof(GLfloat) * lightingArr.size());
	lightingVbo_->Bind();
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)0); // encoded lighting
	glEnableVertexAttribArray(1);

	vertexCount_ = encodedStuffArr.size();
	indexCount_ = tIndices.size();
	tIndices.clear();
	encodedStuffArr.clear();
	lightingArr.clear();

	mtx.unlock();
}
