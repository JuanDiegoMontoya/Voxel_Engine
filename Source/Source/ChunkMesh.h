#pragma once

class VAO;
class VBO;
class IBO;

class ChunkMesh
{
public:

	void BuildMesh(ChunkPtr);
	void BuildBuffers();

	// debug
	static inline bool debug_ignore_light_level = false;
	static inline std::atomic<double> accumtime = 0;
	static inline std::atomic<unsigned> accumcount = 0;
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

	ChunkPtr nearChunks[6];

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

	std::mutex mtx;
};