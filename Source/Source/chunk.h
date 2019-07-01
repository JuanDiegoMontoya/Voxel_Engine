#pragma once
#include "block.h"

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

//typedef class Block;
class VAO;
class VBO;

struct localpos
{
	localpos(glm::ivec3 chunk, glm::ivec3 block)
		: chunk_pos(chunk), block_pos(block) {}
	glm::ivec3 chunk_pos; // within world
	glm::ivec3 block_pos; // within chunk
};

typedef struct Chunk
{
private:
	struct ivec3Hash
	{
		// condenses an ivec3 into a singular number that can be used in a hash
		size_t operator()(glm::ivec3 vec) const
		{
			return ID3D(vec.x, vec.y, vec.z, Chunk::GetChunkSize(), Chunk::GetChunkSize());
		}
	};
	struct ivec3KeyEq
	{
		bool operator()(glm::ivec3 first, glm::ivec3 second) const
		{
			return first == second;
		}
	};
public:
	Chunk(bool active = false)
		: active_(active) {}
	~Chunk();

	void Update();
	void Render();

	// may need to upgrade to glm::i64vec3 if worldgen at far distances is fug'd
	// "origin" chunk goes from 0-CHUNK_SIZE rather than -CHUNK_SIZE/2-CHUNK_SIZE/2
	// chunk at (0,0,0) spans 0-CHUNK_SIZE
	inline static localpos worldBlockToLocalPos(glm::ivec3 worldPos)
	{
		return localpos(worldPos / CHUNK_SIZE, worldPos % CHUNK_SIZE);
	}

	// gives the true world position of a block within a chunk
	inline glm::ivec3 chunkBlockToWorldPos(glm::ivec3 localPos)
	{
		return glm::ivec3(localPos + (pos_ * CHUNK_SIZE));
	}

	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 16;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	static std::unordered_map<glm::ivec3, ChunkPtr, ivec3Hash> chunks;
private:
	void buildMesh();
	std::vector<glm::vec3> buildSingleBlockFaceBologna(glm::ivec3 near, int low, int high, int x, int y, int z);

	glm::ivec3 pos_; // position relative to other chunks (1 chunk = 1 index)
	bool active_;

	// rendering stuff
	VAO* vao_ = nullptr;
	VBO* vbo_ = nullptr;
	size_t vertexCount_; // number of vertices composing the mesh of the chunk
}Chunk, *ChunkPtr;