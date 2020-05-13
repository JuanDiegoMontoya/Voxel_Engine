#pragma once
class ChunkVBOAllocator;

namespace ChunkRenderer
{
	void InitAllocator();
	void GenerateDrawCommands();
	void Render();

	inline std::unique_ptr<ChunkVBOAllocator> allocator;
	inline std::unique_ptr<ChunkVBOAllocator> posAllocator;
}