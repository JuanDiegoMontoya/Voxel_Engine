#pragma once

class ChunkStorage
{
public:

private:
	static inline Concurrency::concurrent_unordered_map // TODO: make CustomGrow(tm) concurrent map solution for portability
		<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks;
};