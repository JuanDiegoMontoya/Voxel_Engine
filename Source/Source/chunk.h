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
		size_t operator()(const glm::ivec3& vec) const
		{
			//return ID3D(vec.x, vec.y, vec.z, Chunk::GetChunkSize(), Chunk::GetChunkSize());
			using std::size_t;
			using std::hash;

			return ((vec.x * 5209) ^ (vec.y * 1811)) ^ (vec.z * 7297);
		}
	};
	struct ivec3KeyEq
	{
		bool operator()(const glm::ivec3& first, const glm::ivec3& second) const
		{
			return first == second;
		}
	};
public:
	Chunk(bool active = false);
	~Chunk();

	void Update();
	void Render();
	void BuildBuffers();

	inline void SetPos(glm::ivec3 pos)
	{
		pos_ = pos;
		model_ = glm::translate(glm::mat4(1.f), glm::vec3(pos_) * (float)CHUNK_SIZE);
	}
	inline const glm::ivec3& GetPos() { return pos_; }

	inline void SetActive(bool e) { active_ = e; }

	// may need to upgrade to glm::i64vec3 if worldgen at far distances is fug'd
	// "origin" chunk goes from 0-CHUNK_SIZE rather than -CHUNK_SIZE/2-CHUNK_SIZE/2
	// chunk at (0,0,0) spans 0-CHUNK_SIZE
	inline static localpos worldBlockToLocalPos(glm::ivec3 worldPos)
	{
		glm::ivec3 chk = glm::floor(glm::vec3(worldPos) / (float)CHUNK_SIZE);// *(float)CHUNK_SIZE;
		glm::ivec3 mod(worldPos % CHUNK_SIZE);
		mod = glm::vec3(
			mod.x >= 0 ? mod.x : CHUNK_SIZE + mod.x,
			mod.y >= 0 ? mod.y : CHUNK_SIZE + mod.y,
			mod.z >= 0 ? mod.z : CHUNK_SIZE + mod.z);
		return localpos(chk, mod);
	}

	// gives the true world position of a block within a chunk
	inline glm::ivec3 chunkBlockToWorldPos(glm::ivec3 localPos)
	{
		return glm::ivec3(localPos + (pos_ * CHUNK_SIZE));
	}

	inline Block& At(glm::ivec3 p)
	{
		return blocks[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	inline Block& At(int x, int y, int z)
	{
		return blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 64;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	static Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, ivec3Hash> chunks;
private:
	void buildMesh();
	std::vector<float> buildSingleBlockFace(
		const glm::ivec3& nearFace,
		int quadStride, int curQuad, const float* data,
		const glm::ivec3& blockPos);

	glm::mat4 model_;
	glm::ivec3 pos_; // position relative to other chunks (1 chunk = 1 index)
	bool active_;

	// rendering stuff
	glm::vec4 color; // temp
	VAO* vao_ = nullptr;
	VBO* vbo_ = nullptr;

	// temporary buffer(s)
	std::vector<float> vertices; // everything buffer

	//std::vector<glm::vec3> vtxPosBuffer; // positions
	//std::vector<glm::vec2> vtxTexBuffer; // texture UVs
	//std::vector<glm::vec3> vtxNmlBuffer; // normals
	//std::vector<glm::vec3> vtxTanBuffer; // tangents
	//std::vector<glm::vec3> vtxBitBuffer; // bitangents
	size_t vertexCount_ = 0; // number of vertices composing the mesh of the chunk
}Chunk, *ChunkPtr;