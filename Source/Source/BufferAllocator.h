#pragma once

class BufferAllocator
{
public:

	BufferAllocator(GLsizei size, GLsizei alignment);
	~BufferAllocator();

	uint64_t Allocate(void* data, GLsizei size, void* userdata = NULL);
	bool Free(uint64_t handle);
	bool FreeOldest();
	GLuint GetGPUHandle() { return gpuHandle; }
	const auto& GetAllocs() { return allocs_; }
	GLuint ActiveAllocs() { return numActiveAllocs_; }

	const GLsizei align_;

	struct allocationData
	{
		uint64_t handle;// "pointer"
		GLsizei offset; // offset from beginning of this memory
		GLsizei size;   // allocation size
		GLdouble time;  // time of allocation
		void* userdata; // user-defined data
	};

private:
	std::vector<allocationData> allocs_;
	using Iterator = decltype(allocs_.begin());

	// merges adjacent null allocations to iterator
	void maybeMerge(Iterator it);

	// verifies the buffer has no errors, debug only
	void dbgVerify();

	GLuint gpuHandle;
	uint64_t nextHandle = 1;
	GLuint numActiveAllocs_ = 0;
};