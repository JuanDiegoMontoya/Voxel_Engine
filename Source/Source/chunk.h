#pragma once
#include "block.h"

// cubic dimensions of a single chunk
//#define CHUNK_SIZE 16

//typedef class Block;
class VAO;
class VBO;

struct localpos
{
	localpos(glm::ivec3& chunk, glm::ivec3& block)
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

	glm::vec3 colorTEMP;

	void Update();
	void Render();
	void BuildBuffers();
	void BuildMesh();

	inline const glm::mat4& GetModel() const { return model_; }

	inline void SetPos(const glm::ivec3& pos)
	{
		pos_ = pos;
		model_ = glm::translate(glm::mat4(1.f), glm::vec3(pos_) * (float)CHUNK_SIZE);
	}
	inline const glm::ivec3& GetPos() { return pos_; }

	inline void SetActive(bool e) { active_ = e; }
	inline bool IsActive() { return active_; }

	// may need to upgrade to glm::i64vec3 if worldgen at far distances is fug'd
	// "origin" chunk goes from 0-CHUNK_SIZE rather than -CHUNK_SIZE/2-CHUNK_SIZE/2
	// chunk at (0,0,0) spans 0-CHUNK_SIZE
	inline static localpos worldBlockToLocalPos(const glm::ivec3 worldPos)
	{
		glm::ivec3 chk = glm::floor(glm::vec3(worldPos) / (float)CHUNK_SIZE);// *(float)CHUNK_SIZE;
		glm::ivec3 mod(worldPos % CHUNK_SIZE);
		mod = glm::vec3(
			mod.x >= 0 ? mod.x : CHUNK_SIZE + mod.x,
			mod.y >= 0 ? mod.y : CHUNK_SIZE + mod.y,
			mod.z >= 0 ? mod.z : CHUNK_SIZE + mod.z);
		return localpos(chk, mod);
		
		//glm::ivec3 chk;// = worldPos >> glm::sqrt(CHUNK_SIZE);
		//chk.x = worldPos.x >> glm::log2<int>(CHUNK_SIZE);
		//chk.y = worldPos.y >> glm::log2<int>(CHUNK_SIZE);
		//chk.z = worldPos.z >> glm::log2<int>(CHUNK_SIZE);
		//glm::ivec3 mod;// = worldPos % CHUNK_SIZE;
		//mod.x = worldPos.x & CHUNK_SIZE;
		//mod.y = worldPos.y & CHUNK_SIZE;
		//mod.z = worldPos.z & CHUNK_SIZE;
		//return localpos(chk, mod);
	}

	// gives the true world position of a block within a chunk
	inline glm::ivec3 chunkBlockToWorldPos(const glm::ivec3 local)
	{
		return glm::ivec3(local + (pos_ * CHUNK_SIZE));
	}

	inline Block& At(const glm::ivec3 p)
	{
		return blocks[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	inline Block& At(int x, int y, int z)
	{
		return blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	// block at a position in world space
	inline static BlockPtr AtWorld(const glm::ivec3 p)
	{
		localpos w = worldBlockToLocalPos(p);
		ChunkPtr cnk = chunks[w.chunk_pos];
		if (cnk)
			return &cnk->At(w.block_pos);
		return nullptr;
	}

	inline glm::vec3 GetMin() const
	{
		return glm::vec3(pos_ * CHUNK_SIZE);
	}

	inline glm::vec3 GetMax() const
	{
		return glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE - 1);
	}

	inline bool IsVisible() const { return visible_; }
	inline void SetVisible(bool b) { visible_ = b; }

	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 32;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	friend class Level;
	static Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, ivec3Hash> chunks;
private:

	void buildBlockVertices(
		const glm::ivec3& pos,
		const float* data,
		int quadStride,
		const Block& block);
	void buildSingleBlockFace(
		const glm::ivec3& nearFace,
		int quadStride, int curQuad, const float* data,
		const glm::ivec3& blockPos,
		const Block& block);

	glm::mat4 model_;
	glm::ivec3 pos_; // position relative to other chunks (1 chunk = 1 index)
	bool active_;
	bool visible_;

	// rendering stuff
	VAO* vao_ = nullptr;
	VBO* positions_ = nullptr;
	VBO* normals_ = nullptr;
	VBO* colors_ = nullptr;
	VBO* speculars_ = nullptr;
	//IBO* ibo_ = nullptr;

	// temporary buffer(s)
	std::vector<glm::vec3> tPositions;
	std::vector<glm::vec3> tNormals;
	std::vector<glm::vec3> tColors;
	std::vector<float> tSpeculars;
	std::vector<GLubyte> indices; // probably finna be unused
	//std::vector<glm::vec3> vtxPosBuffer; // positions
	//std::vector<glm::vec2> vtxTexBuffer; // texture UVs
	//std::vector<glm::vec3> vtxNmlBuffer; // normals
	//std::vector<glm::vec3> vtxTanBuffer; // tangents
	//std::vector<glm::vec3> vtxBitBuffer; // bitangents
	size_t vertexCount_ = 0; // number of vertices composing the mesh of the chunk
}Chunk, *ChunkPtr;

void TestCoordinateStuff();