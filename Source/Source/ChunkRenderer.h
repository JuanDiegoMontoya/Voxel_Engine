#pragma once
#include "BufferAllocator.h"
//class BufferAllocator;

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommands();
	void Render();

	inline std::unique_ptr<BufferAllocator> allocator;
}