#pragma once
#include "block.h"
//#include "chunk.h"
#include "NuRenderer.h"
#include <dib.h>
#include "ChunkVBOAllocator.h"

class VAO;
class VBO;
class IBO;
class DIB;
struct Chunk;

class ChunkMesh
{
public:

	void Render();
	void RenderSplat();
	void BuildBuffers();
	void BuildMesh();
	void SetParent(Chunk*);

	GLsizei GetVertexCount() { return vertexCount_; }
	GLsizei GetIndexCount() { return indexCount_; }
	GLsizei GetPointCount() { return pointCount_; }

	// debug
	static inline bool debug_ignore_light_level = false;
	static inline std::atomic<double> accumtime = 0;
	static inline std::atomic<unsigned> accumcount = 0;
private:

	void buildBlockFace(
		int face,
		const glm::ivec3& blockPos,
		BlockType block);
	void addQuad(const glm::ivec3& lpos, BlockType block, int face, Chunk* nearChunk, Light light);
	int vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm);


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

	Chunk* parent = nullptr;
	Chunk* nearChunks[6];

	std::unique_ptr<IBO> ibo_;
	std::unique_ptr<VAO> vao_;
	std::unique_ptr<VBO> encodedStuffVbo_;
	std::unique_ptr<VBO> lightingVbo_;
	std::unique_ptr<VBO> posVbo_;

	// vertex data (held until buffers are sent to GPU)
	std::vector<GLuint> tIndices;
	std::vector<GLfloat> encodedStuffArr;
	std::vector<GLfloat> lightingArr;

	GLsizei vertexCount_ = 0; // number of block vertices
	GLsizei indexCount_ = 0; // number of block vertices


	// SPLATTING STUFF
	std::unique_ptr<VAO> svao_;
	std::unique_ptr<VBO> svbo_;
	std::vector<GLfloat> sPosArr; // point positions (optimize later)
	GLsizei pointCount_ = 0;
	bool voxelReady_ = true; // hack to prevent same voxel from being added multiple times

	// indirect drawing stuff
	std::unique_ptr<DIB> dib_;

	std::shared_mutex mtx;
};


