#pragma once
#include <camera.h>
#include <Frustum.h>
#include "block.h"
#include "light.h"
#include "biome.h"
#include "misc_utils.h"
#include <mutex>
#include <concurrent_unordered_map.h> // TODO: temp solution to concurrent chunk access
#include <atomic>
#include <Shapes.h>

#include <cereal/archives/binary.hpp>

#define MARCHED_CUBES 0
#define ID3D(x, y, z, h, w) (x + h * (y + w * z))
#define ID2D(x, y, w) (w * y + x)

//typedef class Block;

class VAO;
class VBO;
class IBO;

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

// TODO: clean this up a lot
typedef struct Chunk
{
private:
public:
	Chunk();
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
		bounds.min = glm::vec3(pos_ * CHUNK_SIZE);
		bounds.max = glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE - 0);
	}

	const glm::ivec3& GetPos() { return pos_; }
	bool IsVisible(Camera& cam) const
	{
		return cam.GetFrustum()->IsInside(bounds) >= Frustum::Visibility::Partial;
	}


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

	inline Block BlockAtCheap(const glm::ivec3 p)
	{
		return blocks[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	Light& LightAt(const glm::ivec3 p)
	{
		return lightMap[ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE)];
	}

	Light LightAtCheap(const glm::ivec3 p)
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

	AABB GetAABB() const
	{
		return bounds;
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
	static inline std::atomic<double> accumtime = 0;
	static inline std::atomic<unsigned> accumcount = 0;


	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		// save position, block, and lighting data
		ar(pos_, cereal::binary_data(blocks, sizeof(blocks)), cereal::binary_data(lightMap, sizeof(lightMap)));
	}

	friend class WorldGen;
	friend class ChunkManager;
	//friend class Renderer;

private:
	enum
	{
		Far,
		Near,
		Left,
		Right,
		Top,
		Bottom,

		fCount
	};

	static inline const glm::ivec3 faces[6] =
	{
		{ 0, 0, 1 }, // 'far' face    (-z direction)
		{ 0, 0,-1 }, // 'near' face   (+z direction)
		{-1, 0, 0 }, // 'left' face   (-x direction)
		{ 1, 0, 0 }, // 'right' face  (+x direction)
		{ 0, 1, 0 }, // 'top' face    (+y direction)
		{ 0,-1, 0 }, // 'bottom' face (-y direction)
	};

	void buildBlockVertices_normal(
		const glm::ivec3& pos,
		Block block);
	void buildBlockFace(
		int face,
		const glm::ivec3& blockPos,
		Block block);

	// adds quad at given location
	void addQuad(const glm::ivec3& lpos, Block block, 
		int face, ChunkPtr nearChunk, Light light);

	// returns inverse of occludedness
	float computeBlockAO(
		Block block,
		const glm::ivec3& blockPos,
		const glm::vec3& corner,
		const glm::ivec3& nearFace);
	int vertexFaceAO(
		const glm::vec3& lpos,
		const glm::vec3& cornerDir,
		const glm::vec3& norm);

	std::mutex mutex_;// used for safe concurrent access
	std::mutex vertex_buffer_mutex_;

	// buffers
	std::unique_ptr<IBO> ibo_;
	std::unique_ptr<VAO> vao_;
	std::unique_ptr<VBO> encodedStuffVbo_;
	std::unique_ptr<VBO> lightingVbo_;
	// vertex data (held until buffers are sent to GPU)
	std::vector<GLuint> tIndices;
	std::vector<GLfloat> encodedStuffArr;
	std::vector<GLfloat> lightingArr;
	GLsizei vertexCount_ = 0; // number of block vertices
	GLsizei indexCount_ = 0; // number of block vertices

	glm::mat4 model_;
	glm::ivec3 pos_;	// position relative to other chunks (1 chunk = 1 index)
	bool visible_;		// used in frustum culling
	AABB bounds{};
	ChunkPtr nearChunks[6];
}Chunk, *ChunkPtr;

void TestCoordinateStuff();


inline const glm::vec3 normals[] =
{
	{ 0, 0, 1 }, // 'far' face    (+z direction)
	{ 0, 0,-1 }, // 'near' face   (-z direction)
	{-1, 0, 0 }, // 'left' face   (-x direction)
	{ 1, 0, 0 }, // 'right' face  (+x direction)
	{ 0, 1, 0 }, // 'top' face    (+y direction)
	{ 0,-1, 0 }  // 'bottom' face (-y direction)
};

// clockwise from bottom left texture coordinates
inline const glm::vec2 tex_corners[] =
{
	{ 0, 0 },
	{ 0, 1 },
	{ 1, 1 },
	{ 1, 0 }
};
inline void Decode(GLuint encoded, glm::uvec3& modelPos, glm::vec3& normal, glm::vec2& texCoord)
{
	// decode vertex position
	modelPos.x = encoded >> 26;
	modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
	modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111
	//modelPos += 0.5;

	// decode normal
	GLuint normalIdx = (encoded >> 11) & 0x7; // = 0b111
	normal = normals[normalIdx];

	// decode texture index and UV
	GLuint textureIdx = (encoded >> 2) & 0x1FF; // = 0b1111111111
	GLuint cornerIdx = (encoded >> 0) & 0x3; // = 0b11
	glm::vec2 corner = tex_corners[cornerIdx];

	// sample from texture using knowledge of texture dimensions and block index
	// texCoord = ...
}

inline GLuint Encode(const glm::uvec3& modelPos, GLuint normalIdx, GLuint texIdx, GLuint cornerIdx)
{
	//ASSERT(glm::all(glm::lessThan(modelPos, glm::uvec3(1 << 6))));
	//ASSERT(normalIdx < (1 << 3));
	//ASSERT(texIdx < (1 << 9));
	//ASSERT(cornerIdx < (1 << 2));
	GLuint encoded = 0;

	// encode vertex position
	encoded |= modelPos.x << 26;
	encoded |= modelPos.y << 20;
	encoded |= modelPos.z << 14;

	// encode normal
	encoded |= normalIdx << 11;

	// encode texture information
	encoded |= texIdx << 2;
	encoded |= cornerIdx << 0;

	//glm::uvec3 pos;
	//glm::vec3 normal;
	//glm::vec2 texCoord;
	//Decode(encoded, pos, normal, texCoord);
	//ASSERT(glm::all(glm::lessThanEqual(pos, glm::uvec3(32))));
	//ASSERT(pos == modelPos);

	//float ayy = glm::uintBitsToFloat(encoded);
	//GLuint lmao = glm::floatBitsToUint(ayy);
	return encoded;
}