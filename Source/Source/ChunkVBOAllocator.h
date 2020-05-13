#pragma once
#include <dib.h>

class VBO;
struct Chunk;

// TODO: abtract the allocator part of this class into another class
class ChunkVBOAllocator
{
public:
	ChunkVBOAllocator(GLsizei size, GLsizei alignment);
	~ChunkVBOAllocator();

	bool Allocate(Chunk* chunk, void* data, GLsizei size);
	bool Free(Chunk* chunk);
	bool FreeOldest();
	std::unique_ptr<VBO>& GetVBO() { return vbo_; }

	std::vector<DrawArraysIndirectCommand> GetCommands();
	std::vector<glm::vec3> GetChunkPositions();
	//std::unordered_map<glm::ivec3, GLuint> posToIntMap;
	//GLuint posSSBO;

private:
	struct allocationData
	{
		GLsizei offset; // offset from beginning of this memory
		GLsizei size;   // allocation size
		Chunk* chunk;
		GLdouble time;  // time of allocation
	};

	std::vector<allocationData> allocs_;
	using Iterator = decltype(allocs_.begin());

	// merges adjacent null allocations to iterator
	void maybeMerge(Iterator it);

	// verifies the buffer has no errors, debug only
	bool dbgVerify();

	std::unique_ptr<VBO> vbo_;
	const GLsizei align_;
};