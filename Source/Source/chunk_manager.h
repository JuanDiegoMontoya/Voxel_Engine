#pragma once
#include "block.h"

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;

class ChunkManager
{
public:
	ChunkManager();

	// interaction
	void Update(LevelPtr level);
	void UpdateBlock(glm::ivec3 wpos, Block::BlockType t);

	// getters
	float GetLoadDistance() { return loadDistance_; }
	float GetUnloadLeniency() { return unloadLeniency_; }
	unsigned GetMaxLoadPerFrame() { return maxLoadPerFrame_; }

	// setters
	void SetLoadDistance(float d) { loadDistance_ = d; }
	void SetUnloadLeniency(float d) { unloadLeniency_ = d; }
	void SetMaxLoadPerFrame(unsigned n) { maxLoadPerFrame_ = n; }
private:
	// functions
	void ProcessUpdatedChunks();
	bool isChunkInUpdateList(ChunkPtr chunk);
	void checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near);

	void createNearbyChunks(); // and delete far away chunks
	void generateNewChunks();

	// vars
	float loadDistance_;
	float unloadLeniency_;
	unsigned maxLoadPerFrame_;
	std::vector<ChunkPtr> updatedChunks_;
	std::vector<ChunkPtr> genChunkList_;
};