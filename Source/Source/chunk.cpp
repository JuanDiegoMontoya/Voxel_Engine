#include "stdafx.h"
#include "vbo.h"
#include "vao.h"
#include "chunk.h"
#include "block.h"
#include "camera.h"

std::unordered_map<glm::ivec3, ChunkPtr, Chunk::ivec3Hash> Chunk::chunks;

Chunk::~Chunk()
{
	delete vao_;
	delete vbo_;
}

void Chunk::Update()
{
	//// update all blocks within the chunk
	//for (int x = 0; x < CHUNK_SIZE; x++)
	//{
	//	for (int y = 0; y < CHUNK_SIZE; y++)
	//	{
	//		for (int z = 0; z < CHUNK_SIZE; z++)
	//		{
	//			blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].Update();
	//		}
	//	}
	//}

	// build mesh after ensuring culled blocks are culled
	buildMesh();
}

void Chunk::Render()
{
}

void Chunk::buildMesh()
{
	Camera* cam = Render::GetCamera();
	vao_->Bind();
	delete vbo_;
	std::vector<glm::vec3> vertices;
	vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 6 * 3); // one entire side of a chunk (assumed flat)
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				// skip fully transparent blocks
				if (blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)].GetType() == Block::bAir)
					continue;

				// check if each face would be obscured, and adds the ones that aren't to the vbo
				// obscured IF side is adjacent to opaque block
				// NOT obscured if side is adjacent to nothing or transparent block
				std::vector<glm::vec3> temp;

				// back
				temp = buildSingleBlockFaceBologna(glm::vec3(x, y, z - 1), 0, 18, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// front
				temp = buildSingleBlockFaceBologna(glm::vec3(x, y, z + 1), 18, 36, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// left
				temp = buildSingleBlockFaceBologna(glm::vec3(x - 1, y, z), 36, 54, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// right
				temp = buildSingleBlockFaceBologna(glm::vec3(x + 1, y, z), 54, 72, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// bottom
				temp = buildSingleBlockFaceBologna(glm::vec3(x, y - 1, z), 72, 90, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());

				// top
				temp = buildSingleBlockFaceBologna(glm::vec3(x, y + 1, z), 90, 108, x, y, z);
				vertices.insert(vertices.end(), temp.begin(), temp.end());
			}
		}
	}
	vbo_ = new VBO(&vertices[0], sizeof(glm::vec3) * vertices.size(), GL_STATIC_DRAW);
}

std::vector<glm::vec3> Chunk::buildSingleBlockFaceBologna(glm::ivec3 near, int low, int high, int x, int y, int z)
{
	std::vector<glm::vec3> quad(6);

	localpos zNeg = worldBlockToLocalPos(chunkBlockToWorldPos(near));
	if (chunks[zNeg.chunk_pos]->active_ &&
		chunks[zNeg.chunk_pos]->blocks[ID3D(
			zNeg.block_pos.x, zNeg.block_pos.y, zNeg.block_pos.z, CHUNK_SIZE, CHUNK_SIZE)].GetType()
		!= Block::bAir) // back (-Z)
	{
		// transform the vertices relative to the chunk
		// (the full world transformation will be completed in a shader)
		glm::mat4 localTransform = glm::translate(glm::mat4(1.f),
			Utils::mapToRange(glm::vec3(x, y, z), 0.f, (float)CHUNK_SIZE, -1.f, 1.f));
		for (int i = low; i < high; i += 3)
		{
			glm::vec4 tri(Render::cube_vertices[i], Render::cube_vertices[i + 1], Render::cube_vertices[i + 2], 1.f);
			quad.push_back(localTransform * tri);
		}
	}

	return quad;
}
