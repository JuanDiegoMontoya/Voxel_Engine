#pragma once
#include "ChunkHelpers.h"
#include "chunk.h"

class ChunkStorage
{
public:

	static inline ChunkPtr GetChunk(const glm::ivec3& cpos)
	{
		auto it = chunks_.find(cpos);
		if (it != chunks_.end())
			return it->second;
		return nullptr;
	}

	static inline Block AtWorldC(const glm::ivec3& wpos)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
			return cnk->BlockAt(w.block_pos);
		return Block();
	}

	static inline Block AtWorldD(const ChunkHelpers::localpos& p)
	{
		Chunk* cnk = chunks_[p.chunk_pos];
		if (cnk)
			return cnk->BlockAt(p.block_pos);
		return Block();
	}

	static inline auto& GetMapRaw()
	{
		return chunks_;
	}

private:
	static inline Concurrency::concurrent_unordered_map // TODO: make CustomGrow(tm) concurrent map solution for portability
		<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks_;
};