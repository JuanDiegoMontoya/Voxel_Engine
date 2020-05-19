#pragma once
#include "BufferAllocator.h"
#include <Shapes.h>
//class BufferAllocator;

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommands();
	void GenerateDrawCommandsGPU();
	void GenerateDrawCommandsSplat();
	void Render();
	void RenderSplat();

	inline std::unique_ptr<BufferAllocator<AABB16>> allocator;
	inline std::unique_ptr<BufferAllocator<AABB16>> allocatorSplat;
}