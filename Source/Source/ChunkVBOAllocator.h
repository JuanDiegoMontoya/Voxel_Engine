#pragma once

class VBO;
struct Chunk;

class ChunkVBOAllocator
{
public:
	ChunkVBOAllocator(GLsizei size);
	~ChunkVBOAllocator();

	bool Allocate(Chunk* chunk, void* data, GLsizei size);
	bool Free(Chunk* chunk);
	bool FreeOldest();

private:
	struct allocationData
	{
		Chunk* chunk;
		GLsizei offset; // offset from beginning of this memory
		GLsizei size;   // allocation size
		GLdouble time;  // time of allocation
	};

	std::vector<allocationData> allocs_;
	using Iterator = decltype(allocs_.begin());

	// merges adjacent null allocations to iterator
	void maybeMerge(Iterator it);

	std::unique_ptr<VBO> vbo_;
};