#pragma once

template<typename UserDataT>
class BufferAllocator
{
public:

	BufferAllocator(GLsizei size, GLsizei alignment);
	~BufferAllocator();

	uint64_t Allocate(void* data, GLsizei size, UserDataT userdata = {});
	bool Free(uint64_t handle);
	bool FreeOldest();
	GLuint GetGPUHandle() { return gpuHandle; }
	const auto& GetAllocs() { return allocs_; }
	GLuint ActiveAllocs() { return numActiveAllocs_; }

	const GLsizei align_;


	template<typename UT>
	struct allocationData
	{
		allocationData() = default;
		allocationData(UT u) : userdata(u) {}
		uint64_t handle;// "pointer"
		GLsizei offset; // offset from beginning of this memory
		GLsizei size;   // allocation size
		GLdouble time;  // time of allocation
		UT userdata; // user-defined data
	};

	// no userdata specialization
	struct Empty_ {};
	template<>
	struct allocationData<Empty_>
	{
		allocationData() = default;
		allocationData(Empty_) {}
		uint64_t handle;// "pointer"
		GLsizei offset; // offset from beginning of this memory
		GLsizei size;   // allocation size
		GLdouble time;  // time of allocation
	};

private:
	std::vector<allocationData<UserDataT>> allocs_;
	using Iterator = decltype(allocs_.begin());



	// merges adjacent null allocations to iterator
	void maybeMerge(Iterator it);

	// verifies the buffer has no errors, debug only
	void dbgVerify();

	GLuint gpuHandle;
	uint64_t nextHandle = 1;
	GLuint numActiveAllocs_ = 0;
};

#include "BufferAllocation.inl"