#pragma once
#include "BufferAllocator.h"
//class BufferAllocator;

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommands();
	void GenerateDrawCommandsSplat();
	void Render();
	void RenderSplat();

	inline std::unique_ptr<BufferAllocator<void*>> allocator;
	inline std::unique_ptr<BufferAllocator<void*>> allocatorSplat;
}