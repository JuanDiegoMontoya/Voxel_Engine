#include "stdafx.h"
#include "BufferAllocator.h"


BufferAllocator::BufferAllocator(GLsizei size, GLsizei alignment) : align_(alignment)
{
	size += (align_ - (size % align_)) % align_;

	// allocate uninitialized memory in VRAM
	glGenBuffers(1, &gpuHandle);
	glBindBuffer(GL_ARRAY_BUFFER, gpuHandle);
	glNamedBufferData(gpuHandle, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// make one big null allocation
	allocs_.push_back({ NULL, 0, size, 0 });
}


BufferAllocator::~BufferAllocator()
{
	sizeof(BufferAllocator);
	alignof(BufferAllocator);
	glDeleteBuffers(1, &gpuHandle);
}


uint64_t BufferAllocator::Allocate(void* data, GLsizei size)
{
	size += (align_ - (size % align_)) % align_;
	// find smallest NULL allocation that will fit
	Iterator small = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].handle == NULL && allocs_[i].size >= size) // potential allocation
		{
			if (small == allocs_.end())// initialize small
				small = allocs_.begin() + i;
			else if (allocs_[i].size < small->size)
				small = allocs_.begin() + i;
		}
	}

	// allocation failure
	if (small == allocs_.end())
		return NULL;

	// split free allocation
	allocationData newAlloc;
	newAlloc.handle = nextHandle++;
	newAlloc.offset = small->offset;
	newAlloc.size = size;
	newAlloc.time = glfwGetTime();

	small->offset += newAlloc.size;
	small->size -= newAlloc.size;

	// replace shrunk alloc if it would become degenerate
	if (small->size == 0)
		*small = newAlloc;
	else
		allocs_.insert(small, newAlloc);

	glNamedBufferSubData(gpuHandle, newAlloc.offset, newAlloc.size, data);
	ASSERT(dbgVerify());
	return newAlloc.handle;
}


bool BufferAllocator::Free(uint64_t handle)
{
	if (handle == NULL) return false;
	auto it = std::find_if(allocs_.begin(), allocs_.end(), [&](const auto& a) { return a.handle == handle; });
	if (it == allocs_.end()) // failed to free
		return false;

	it->handle = NULL;
	maybeMerge(it);
	ASSERT(dbgVerify());
	return true;
}


bool BufferAllocator::FreeOldest()
{
	// find and free the oldest allocation
	Iterator old = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].handle != NULL)
		{
			if (old == allocs_.end())
				old = allocs_.begin() + i;
			else if (allocs_[i].time < old->time)
				old = allocs_.begin() + i;
		}
	}

	// failed to find old node to free
	if (old == allocs_.end())
		return false;

	old->handle = NULL;
	maybeMerge(old);

	ASSERT(dbgVerify());
	return true;
}


void BufferAllocator::maybeMerge(Iterator it)
{
	bool removeIt = false;
	bool removeNext = false;

	// merge with next alloc
	if (it != allocs_.end() - 1)
	{
		Iterator next = it + 1;
		if (next->handle == NULL)
		{
			it->size += next->size;
			removeNext = true;
		}
	}

	// merge with previous alloc
	if (it != allocs_.begin())
	{
		Iterator prev = it - 1;
		if (prev->handle == NULL)
		{
			prev->size += it->size;
			removeIt = true;
		}
	}

	// erase merged allocations
	if (removeIt && removeNext)
		allocs_.erase(it, it + 2); // this and next
	else if (removeIt)
		allocs_.erase(it);     // just this
	else if (removeNext)
		allocs_.erase(it + 1);         // just next
}


bool BufferAllocator::dbgVerify()
{
	uint64_t prevPtr = 1;
	GLsizei sumSize = 0;
	for (const auto& alloc : allocs_)
	{
		// check there are never two null blocks in a row
		if (prevPtr == NULL && alloc.handle == NULL)
		{
			printf("\nVerify failed: two null blocks in a row!\n");
			ASSERT(0);
			return false;
		}
		prevPtr = alloc.handle;

		// check offset is equal to total size so far
		if (alloc.offset != sumSize)
		{
			printf("\nVerify failed: size/offset discrepancy!\n");
			ASSERT(0);
			return false;
		}
		sumSize += alloc.size;

		// check alignment
		if (alloc.offset % align_ != 0)
		{
			printf("\nVerify failed: block alignment mismatch!\n");
			ASSERT(0);
			return false;
		}

		// check degenerate (0-size) allocation
		if (alloc.size == 0)
		{
			printf("\nVerify failed: 0-size allocation!\n");
			ASSERT(0);
			return false;
		}
	}

	// all checks passed
	return true;
}