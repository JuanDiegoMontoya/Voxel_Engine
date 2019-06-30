#pragma once
#include "block.h"

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

//typedef class Block;
class VAO;
class VBO;

typedef struct Chunk
{
private:
	//static std::unordered_map<glm::ivec3, ChunkPtr> activechunks;
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
	Chunk();
	~Chunk();

	void Update();
	void Render();

	// converts a world position to one that can access the chunk map
	static glm::ivec3 worldToChunkPos();

	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 16;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	static std::unordered_map<glm::ivec3, ChunkPtr, ivec3Hash> activechunks;
private:
	void buildMesh();

	VAO* vao_;
	VBO* vbo_;
}Chunk, *ChunkPtr;