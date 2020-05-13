#include "stdafx.h"
#include "ChunkVBOAllocator.h"
#include <vbo.h>
#include "chunk.h"
#include "ChunkMesh.h"
#include "ChunkStorage.h"

ChunkVBOAllocator::ChunkVBOAllocator(GLsizei size, GLsizei alignment) : align_(alignment)
{
	// allocate uninitialized memory in VRAM
	size += (align_ - (size % align_)) % align_;
	vbo_ = std::make_unique<VBO>(nullptr, size, GL_STATIC_DRAW);

	// make one big null allocation
	allocs_.push_back({ 0, size, nullptr, 0 });

	//// TODO: move this part into out of here
	//// init shader buffer for storing line segments
	//glGenBuffers(1, &posSSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	//// store 100000 (ivec3) positions
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(glm::ivec3), nullptr, GL_STATIC_DRAW);
	//// bind to location 0
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posSSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//const auto& chunkmap = ChunkStorage::GetMapRaw();
	//GLuint posMapIndex = 0;
	//for (auto pair : chunkmap)
	//{
	//	posToIntMap[pair.first] = posMapIndex;
	//	glNamedBufferSubData(posSSBO, posMapIndex * sizeof(glm::ivec3),
	//		sizeof(glm::ivec3), glm::value_ptr(pair.first));
	//	posMapIndex++;
	//}
}


ChunkVBOAllocator::~ChunkVBOAllocator()
{
}


bool ChunkVBOAllocator::Allocate(Chunk* chunk, void* data, GLsizei size)
{
	size += (align_ - (size % align_)) % align_;
	// find smallest NULL allocation that will fit
	Iterator small = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].chunk == nullptr && allocs_[i].size >= size) // potential allocation
		{
			if (small == allocs_.end())// initialize small
				small = allocs_.begin() + i;
			else if (allocs_[i].size < small->size)
				small = allocs_.begin() + i;
		}
	}

	// allocation failure
	if (small == allocs_.end())
		return false;

	// split free allocation
	allocationData newAlloc;
	newAlloc.chunk = chunk;
	newAlloc.offset = small->offset;
	newAlloc.size = size;
	newAlloc.time = glfwGetTime();

	small->offset += newAlloc.size;
	small->size -= newAlloc.size;

	allocs_.insert(small, newAlloc);
	
	glNamedBufferSubData(vbo_->GetID(), newAlloc.offset, newAlloc.size, data);
	ASSERT(dbgVerify());
	return true;
}


bool ChunkVBOAllocator::Free(Chunk* chunk)
{
	if (chunk == nullptr) return false;
	auto it = std::find_if(allocs_.begin(), allocs_.end(), [&](const auto& a) { return a.chunk == chunk; });
	if (it == allocs_.end()) // failed to free
		return false;

	it->chunk = nullptr;
	maybeMerge(it);
	ASSERT(dbgVerify());
	return true;
}


bool ChunkVBOAllocator::FreeOldest()
{
	// find and free the oldest allocation
	Iterator old = allocs_.end();
	for (int i = 0; i < allocs_.size(); i++)
	{
		if (allocs_[i].chunk != nullptr)
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

	old->chunk = nullptr;
	maybeMerge(old);

	ASSERT(dbgVerify());
	return true;
}


void ChunkVBOAllocator::maybeMerge(Iterator it)
{
	bool removeIt = false;
	bool removeNext = false;
	
	// merge with next alloc
	if (it != allocs_.end() - 1)
	{
		Iterator next = it + 1;
		if (next->chunk == nullptr)
		{
			it->size += next->size;
			removeNext = true;
		}
	}

	// merge with previous alloc
	if (it != allocs_.begin())
	{
		Iterator prev = it - 1;
		if (prev->chunk == nullptr)
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


std::vector<DrawArraysIndirectCommand> ChunkVBOAllocator::GetCommands()
{
	std::vector<DrawArraysIndirectCommand> commands;
	int baseInstance = 0;

	for (const allocationData& alloc : allocs_)
	{
		if (alloc.chunk != nullptr)
		{
			DrawArraysIndirectCommand cmd;
			cmd.count = alloc.chunk->GetMesh().GetVertexCount();
			cmd.instanceCount = 1;
			cmd.first = alloc.offset / align_;
			//cmd.first = alloc.offset;
			//cmd.baseInstance = baseInstance++;
			//cmd.baseInstance = 0;
			//cmd.baseInstance = posToIntMap[alloc.chunk->GetPos()];
			cmd.baseInstance = cmd.first; // same stride as vertices technically
			commands.push_back(cmd);
		}
	}

	return commands;
}


std::vector<glm::vec3> ChunkVBOAllocator::GetChunkPositions()
{
	std::vector<glm::vec3> res;
	for (const auto& alloc : allocs_)
	{
		if (alloc.chunk != nullptr)
			res.push_back(alloc.chunk->GetPos());
	}
	return res;
}


bool ChunkVBOAllocator::dbgVerify()
{
	void* prevPtr = (void*)1;
	GLsizei sumSize = 0;
	for (const auto& alloc : allocs_)
	{
		// check there are never two null blocks in a row
		if (prevPtr == nullptr && alloc.chunk == nullptr)
			return false;
		prevPtr = alloc.chunk;

		// check offset is equal to total size so far
		if (alloc.offset != sumSize)
			return false;
		sumSize += alloc.size;

		// check alignment
		if (alloc.offset % align_ != 0)
			return false;
	}

	// all checks passed
	return true;
}