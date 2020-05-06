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

#include "BlockStorage.h"
#include "ChunkHelpers.h"

#include <cereal/archives/binary.hpp>

#define MARCHED_CUBES 0
#define ID3D(x, y, z, h, w) (x + h * (y + w * z))
#define ID2D(x, y, w) (w * y + x)

//typedef class Block;

class VAO;
class VBO;
class IBO;

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
	void BuildMesh();
	void BuildBuffers();


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

	inline Block BlockAt(const glm::ivec3& p)
	{
		return storage.GetBlock(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
	}

	inline Block BlockAt(int index)
	{
		return storage.GetBlock(index);
	}

	inline void SetBlockTypeAt(const glm::ivec3& lpos, BlockType type)
	{
		storage.SetBlock(
			ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE), type);
	}

	inline void SetLightAt(const glm::ivec3& lpos, Light light)
	{
		storage.SetLight(
			ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE), light);
	}

	AABB GetAABB() const
	{
		return bounds;
	}



	// Serialization
	template <class Archive>
	void serialize(Archive& ar)
	{
		// save position, block, and lighting data
		//ar(pos_, cereal::binary_data(blocks, sizeof(blocks)), cereal::binary_data(lightMap, sizeof(lightMap)));
	}

private:


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

	//ArrayBlockStorage storage;
	PaletteBlockStorage storage;
}Chunk, *ChunkPtr;