#pragma once
#include "block.h"
//#include "chunk.h"

class VAO;
class VBO;
class IBO;
struct Chunk;

class ChunkMesh
{
public:

	void Render();
	void RenderSplat();
	void BuildBuffers();
	void BuildMesh();
	void SetParent(Chunk*);

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

	std::shared_mutex mtx;
};


