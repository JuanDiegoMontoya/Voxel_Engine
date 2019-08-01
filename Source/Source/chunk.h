#pragma once
#include "block.h"
#include "biome.h"
#include "misc_utils.h"

#define MARCHED_CUBES (false)

//typedef class Block;
class VAO;
class VBO;

struct localpos
{
	localpos(const glm::ivec3& chunk, const glm::ivec3& block)
		: chunk_pos(chunk), block_pos(block) {}
	glm::ivec3 chunk_pos; // within world
	glm::ivec3 block_pos; // within chunk
};

/*
	0: -x-y+z
	1: +x-y+z
	2: +x-y-z
	3: -x-y-z
	4: -x+y+z
	5: +x+y+z
	6: +x+y-z
	7: -x+y-z
*/
typedef struct
{
	glm::vec3 p[8];
	double val[8]; // density values
} cell;

typedef struct
{
	glm::vec3 ps[3];
} tri;

typedef struct Chunk
{
private:
public:
	Chunk(bool active = false);
	~Chunk();

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
		return glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE - 0);
	}

	inline bool IsVisible() const { return visible_; }
	inline void SetVisible(bool b) { visible_ = b; }

	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 32;

	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	friend class WorldGen;
	friend class ChunkManager;
	static std::unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;

private:
	//static Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;

	void buildBlockVertices_normal(
		const glm::ivec3& pos,
		const float* data,
		int quadStride,
		const Block& block);
	void buildSingleBlockFace(
		const glm::ivec3& nearFace,
		int quadStride, int curQuad, const float* data,
		const glm::ivec3& blockPos,
		const Block& block);

	void buildBlockVertices_marched_cubes(
		const glm::ivec3& pos,
		const Block& block);
	int polygonize(const glm::ivec3& pos);
	cell buildCellFromVoxel(const glm::vec3& wpos);
	glm::vec3 VertexInterp(double isolevel, glm::vec3 p1, glm::vec3 p2, double valp1, double valp2);
	//glm::vec3 VertexInterp2(glm::vec3 p1, glm::vec3 p2, double value);

	/*
		Used for marching cubes. Determines the minimum density of a point
		for it to be considered solid or not.
	*/
	static double isolevel; // between 0 and 1

	glm::mat4 model_;
	glm::ivec3 pos_; // position relative to other chunks (1 chunk = 1 index)
	bool active_;
	bool visible_; // used in frustum culling
	bool generate_ = false; // if the chunks needs to be generated

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
	std::vector<glm::vec4> tColors;
	std::vector<float> tSpeculars;
	std::vector<GLubyte> indices; // probably finna be unused
	//std::vector<glm::vec3> vtxTanBuffer; // tangents
	//std::vector<glm::vec3> vtxBitBuffer; // bitangents
	size_t vertexCount_ = 0; // number of vertices composing the mesh of the chunk
}Chunk, *ChunkPtr;

void TestCoordinateStuff();