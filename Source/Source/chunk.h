#pragma once
#include "block.h"
#include "biome.h"
#include "misc_utils.h"
#include <mutex>
#include <concurrent_unordered_map.h> // temp solution to concurrent chunk access
#include <atomic>

#define MARCHED_CUBES 0

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
	glm::vec3 p[8]; // corner positions
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
	Chunk(bool active = true);
	~Chunk();

	/*################################
						Global Chunk Info
	################################*/
	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE = 32;


	/*################################
						Draw Functions
	################################*/
	void Update();
	void Render();
	void RenderWater();
	void BuildBuffers();
	void BuildMesh();


	/*################################
					Status Functions
	################################*/
	const glm::mat4& GetModel() const { return model_; }

	void SetPos(const glm::ivec3& pos)
	{
		pos_ = pos;
		model_ = glm::translate(glm::mat4(1.f), glm::vec3(pos_) * (float)CHUNK_SIZE);
	}

	const glm::ivec3& GetPos() { return pos_; }
	void SetActive(bool e) { active_ = e; }
	bool IsActive() { return active_; }
	bool IsVisible() const { return visible_; }
	void SetVisible(bool b) { visible_ = b; }


	/*################################
					Coordinate Functions
	################################*/
	// may need to upgrade to glm::i64vec3 if worldgen at far distances is fug'd
	// "origin" chunk goes from 0-CHUNK_SIZE rather than -CHUNK_SIZE/2-CHUNK_SIZE/2
	// chunk at (0,0,0) spans 0-CHUNK_SIZE
	static localpos worldBlockToLocalPos(const glm::ivec3 worldPos)
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
	glm::ivec3 chunkBlockToWorldPos(const glm::ivec3 local)
	{
		return glm::ivec3(local + (pos_ * CHUNK_SIZE));
	}

	Block& At(const glm::ivec3 p)
	{
		return blocks[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	Block& At(int x, int y, int z)
	{
		return blocks[ID3D(x, y, z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	// block at a position in world space
	static BlockPtr AtWorld(const glm::ivec3 p)
	{
		localpos w = worldBlockToLocalPos(p);
		ChunkPtr cnk = chunks[w.chunk_pos];
		if (cnk)
			return &cnk->At(w.block_pos);
		return nullptr;
	}

	glm::vec3 GetMin() const
	{
		return glm::vec3(pos_ * CHUNK_SIZE);
	}

	glm::vec3 GetMax() const
	{
		return glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE - 0);
	}


	/*################################
						Raw Block Data
	################################*/
	Block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	//static inline std::unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;
	static inline Concurrency::concurrent_unordered_map // TODO: make CustomGrow(tm) concurrent map solution to be more portable
		<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;

	friend class WorldGen;
	friend class ChunkManager;
	friend class Renderer;

	static cell buildCellFromVoxel(const glm::vec3& wpos);

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
		const Block& block,
		bool force = false);

	// returns inverse of occludedness
	float computeBlockAO(
		Block block,
		const glm::ivec3& blockPos,
		const glm::vec3& corner,
		const glm::ivec3& nearFace);

	void buildBlockVertices_marched_cubes(
		const glm::ivec3& pos,
		const Block& block);
	size_t polygonize(const glm::ivec3& pos, const Block&);
	glm::vec3 VertexInterp(double isolevel, glm::vec3 p1, glm::vec3 p2, double valp1, double valp2);
	//glm::vec3 VertexInterp2(glm::vec3 p1, glm::vec3 p2, double value);

	/*
		Used for marching cubes. Determines the minimum density of a point
		for it to be considered solid or not.
	*/
	static double isolevel; // between 0 and 1

	glm::mat4 model_;
	glm::ivec3 pos_;			 // position relative to other chunks (1 chunk = 1 index)
	bool active_;					 // unused
	bool visible_;				 // used in frustum culling
	std::mutex mutex_;     // used for safe concurrent access

	// rendering stuff
	VAO* vao_ = nullptr;
	VBO* positions_ = nullptr;
	VBO* normals_ = nullptr;
	VBO* colors_ = nullptr;
	VBO* speculars_ = nullptr;
	//IBO* ibo_ = nullptr;

	// temporary buffer(s)
	std::mutex vertex_buffer_mutex_;
	std::vector<glm::vec3> tPositions;
	std::vector<glm::vec3> tNormals;
	std::vector<glm::vec4> tColors;
	std::vector<float> tSpeculars;

	// water rendering stuff
	VAO* wvao_ = nullptr;
	VBO* wpositions_ = nullptr;
	VBO* wnormals_ = nullptr;
	VBO* wcolors_ = nullptr;
	VBO* wspeculars_ = nullptr;
	std::vector<glm::vec3> wtPositions;
	std::vector<glm::vec3> wtNormals;
	std::vector<glm::vec4> wtColors;
	std::vector<float>		 wtSpeculars;
	size_t wvertexCount_ = 0;

	std::vector<GLubyte> indices; // probably finna be unused
	//std::vector<glm::vec3> vtxTanBuffer; // tangents
	//std::vector<glm::vec3> vtxBitBuffer; // bitangents
	size_t vertexCount_ = 0; // number of vertices composing the mesh of the chunk

	//std::atomic_int useCount
}Chunk, *ChunkPtr;

void TestCoordinateStuff();