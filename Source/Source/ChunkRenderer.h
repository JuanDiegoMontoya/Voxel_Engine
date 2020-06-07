#pragma once
#include "BufferAllocator.h"
#include <Shapes.h>

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommands();
	void GenerateDrawCommandsGPU();
	void GenerateDrawCommandsSplat();
	void GenerateDrawCommandsSplatGPU();
	void Render();
	void RenderSplat();

	void DrawBuffers();

	inline std::unique_ptr<BufferAllocator<AABB16>> allocator;
	inline std::unique_ptr<BufferAllocator<AABB16>> allocatorSplat;

	struct Settings
	{
		// visibility
		float normalMin = 0;
		float normalMax = 800;
		float splatMin = 800;
		float splatMax = 8000;
	}inline settings;
}