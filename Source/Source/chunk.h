#pragma once
#include "block.h"
#include "light.h"
#include "biome.h"
#include "misc_utils.h"
#include <mutex>
#include <concurrent_unordered_map.h> // TODO: temp solution to concurrent chunk access
#include <atomic>

#include <cereal/archives/binary.hpp>

#define MARCHED_CUBES 0
#define ID3D(x, y, z, h, w) (x + h * (y + w * z))
#define ID2D(x, y, w) (w * y + x)

//typedef class Block;

class VAO;
class VBO;

struct localpos
{
	localpos() : chunk_pos(0), block_pos(0) {}
	localpos(const glm::ivec3& chunk, const glm::ivec3& block)
		: chunk_pos(chunk), block_pos(block) {}
	localpos(glm::ivec3&& chunk, glm::ivec3&& block)
		: chunk_pos(std::move(chunk)), block_pos(std::move(block)) {}
	glm::ivec3 chunk_pos; // within world
	glm::ivec3 block_pos; // within chunk
};

//typedef std::pair<glm::ivec3, glm::ivec3> localpos;

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
	Chunk(const Chunk& other);
	Chunk& operator=(const Chunk& rhs);

	/*################################
						Global Chunk Info
	################################*/
	static constexpr int GetChunkSize() { return CHUNK_SIZE; }
	static constexpr int CHUNK_SIZE			  = 32;
	static constexpr int CHUNK_SIZE_SQRED = CHUNK_SIZE * CHUNK_SIZE;
	static constexpr int CHUNK_SIZE_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
	static constexpr int CHUNK_SIZE_LOG2  = 5; // log2(32) = 5


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
	// may need to upgrade parameter to glm::i64vec3 if worldgen at far distances is fug'd
	// "origin" chunk goes from 0-CHUNK_SIZE rather than -CHUNK_SIZE/2-CHUNK_SIZE/2
	// chunk at (0,0,0) spans 0-CHUNK_SIZE
	static localpos worldBlockToLocalPos(const glm::ivec3 wpos)
	{
		localpos ret;
		fastWorldBlockToLocalPos(wpos, ret);
		return ret;
	}

	// improves speed by avoiding local copies and returning
	static void fastWorldBlockToLocalPos(const glm::ivec3& wpos, localpos& ret)
	{
		// compute the modulus of wpos and chunk size (bitwise AND method only works for powers of 2)
		// to find the relative block position in the chunk
		ret.block_pos.x = wpos.x & CHUNK_SIZE - 1;
		ret.block_pos.y = wpos.y & CHUNK_SIZE - 1;
		ret.block_pos.z = wpos.z & CHUNK_SIZE - 1;
		// find the chunk position using integer floor method
		ret.chunk_pos.x = wpos.x / CHUNK_SIZE;
		ret.chunk_pos.y = wpos.y / CHUNK_SIZE;
		ret.chunk_pos.z = wpos.z / CHUNK_SIZE;
		// subtract (floor) if negative w/ non-zero modulus
		if (wpos.x < 0 && ret.block_pos.x) ret.chunk_pos.x--;
		if (wpos.y < 0 && ret.block_pos.y) ret.chunk_pos.y--;
		if (wpos.z < 0 && ret.block_pos.z) ret.chunk_pos.z--;
		// shift local block position forward by chunk size if negative
		if (ret.block_pos.x < 0) ret.block_pos.x += CHUNK_SIZE;
		if (ret.block_pos.y < 0) ret.block_pos.y += CHUNK_SIZE;
		if (ret.block_pos.z < 0) ret.block_pos.z += CHUNK_SIZE;
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

	Light& LightAt(const glm::ivec3 p)
	{
		return lightMap[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
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

	static LightPtr LightAtWorld(const glm::ivec3 p)
	{
		localpos w = worldBlockToLocalPos(p);
		ChunkPtr cnk = chunks[w.chunk_pos];
		if (cnk)
			return &cnk->LightAt(w.block_pos);
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
	Light lightMap[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	//static inline std::unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;
	static inline Concurrency::concurrent_unordered_map // TODO: make CustomGrow(tm) concurrent map solution to be more portable
		<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;
	
	// debug
	static inline bool debug_ignore_light_level = false;

	static cell buildCellFromVoxel(const glm::vec3& wpos);

	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		// save position, block, and lighting data
		ar(pos_, cereal::binary_data(blocks, sizeof(blocks)), cereal::binary_data(lightMap, sizeof(lightMap)));
	}

	friend class WorldGen;
	friend class ChunkManager;
	friend class Renderer;

private:
	void buildBlockVertices_normal(
		glm::ivec3 pos,
		const float* data,
		int quadStride,
		Block block);
	void buildSingleBlockFace(
		glm::ivec3 nearFace,
		int quadStride, int curQuad, const float* data,
		const glm::ivec3& blockPos,
		Block block,
		bool force = false);

	// returns inverse of occludedness
	float computeBlockAO(
		Block block,
		const glm::ivec3& blockPos,
		const glm::vec3& corner,
		const glm::ivec3& nearFace);
	float vertexFaceAO(
		const glm::vec3& corner,
		const glm::ivec3& faceNorm);

	void buildBlockVertices_marched_cubes(
		glm::ivec3 pos,
		Block block);
	size_t polygonize(const glm::ivec3& pos, const Block&);
	glm::vec3 VertexInterp(double isolevel, glm::vec3 p1, glm::vec3 p2, double valp1, double valp2);
	//glm::vec3 VertexInterp2(glm::vec3 p1, glm::vec3 p2, double value);

	/*
		Used for marching cubes. Determines the minimum density of a point
		for it to be considered solid or not.
	*/
	static double isolevel; // between 0 and 1

	glm::mat4 model_;
	glm::ivec3 pos_;	// position relative to other chunks (1 chunk = 1 index)
	bool active_;			// unused
	bool visible_;		// used in frustum culling
	std::mutex mutex_;// used for safe concurrent access

	std::mutex vertex_buffer_mutex_;

	// buffers
	VAO* vao_ = nullptr;
	VBO* positions_ = nullptr;
	VBO* normals_ = nullptr;
	VBO* colors_ = nullptr;
	VBO* speculars_ = nullptr;
	VAO* wvao_ = nullptr;
	VBO* wpositions_ = nullptr;
	VBO* wnormals_ = nullptr;
	VBO* wcolors_ = nullptr;
	VBO* wspeculars_ = nullptr;
	// vertex data (held until buffers are sent to GPU)
	std::vector<glm::vec3> tPositions;
	std::vector<glm::vec3> tNormals;
	std::vector<glm::vec4> tColors;
	std::vector<float> tSpeculars;
	std::vector<glm::vec3> wtPositions;
	std::vector<glm::vec3> wtNormals;
	std::vector<glm::vec4> wtColors;
	std::vector<float>		 wtSpeculars;
	GLsizei wvertexCount_ = 0;// number of transparent block vertices
	GLsizei vertexCount_ = 0; // number of opaque block vertices

}Chunk, *ChunkPtr;

void TestCoordinateStuff();