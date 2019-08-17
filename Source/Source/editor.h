#pragma once
class Renderer;
class ChunkManager;
class Level;

namespace Editor
{
	void Update();

	inline Renderer* renderer;
	inline ChunkManager* chunkManager;
	inline Level* level;
}