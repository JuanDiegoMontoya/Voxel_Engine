#pragma once

// Generic GPU buffer that can store
//   up to 4GB (UINT_MAX) of data
template<typename UserT>
class BufferAllocator
{
public:

	BufferAllocator(GLuint size, GLuint alignment);
	~BufferAllocator();

	uint64_t Allocate(void* data, GLuint size, UserT userdata = {});
	bool Free(uint64_t handle);
	bool FreeOldest();
	GLuint GetGPUHandle() { return gpuHandle; }
	const auto& GetAllocs() { return allocs_; }
	GLuint ActiveAllocs() { return numActiveAllocs_; }

	const GLsizei align_; // allocation alignment

	template<typename UT>
	struct allocationData
	{
		allocationData() = default;
		allocationData(UT u) : userdata(u) {}

		uint64_t handle;// "pointer"
		double time;    // time of allocation
		glm::uvec2 _pad;// GPU padding
		GLuint offset;  // offset from beginning of this memory
		GLuint size;    // allocation size
		UT userdata;    // user-defined data
	};

	// no userdata specialization
	struct Empty_ {};
	template<>
	struct allocationData<Empty_>
	{
		allocationData() = default;
		allocationData(Empty_) {}

		uint64_t handle;// "pointer"
		double time;    // time of allocation
		glm::uvec2 _pad;// GPU padding
		GLuint offset;  // offset from beginning of this memory
		GLuint size;    // allocation size
	};

	GLsizei AllocSize() const { return sizeof(allocationData<UserT>); }

private:
	std::vector<allocationData<UserT>> allocs_;
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