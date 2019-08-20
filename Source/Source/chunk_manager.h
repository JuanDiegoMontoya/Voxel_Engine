#pragma once
#include "chunk_load_manager.h"
#include "block.h"

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;
//class ChunkLoadManager;

class ChunkManager
{
public:
	ChunkManager();

	// interaction
	void Update(LevelPtr level);
	void UpdateBlock(glm::ivec3& wpos, Block::BlockType t, unsigned char writeStrength);
	void UpdateBlockCheap(glm::ivec3& wpos, Block block);
	Block GetBlock(glm::ivec3 wpos); // wrapper function

	// getters
	float GetLoadDistance() const { return loadDistance_; }
	float GetUnloadLeniency() const { return unloadLeniency_; }
	unsigned GetMaxLoadPerFrame() const { return maxLoadPerFrame_; }

	// setters
	void SetCurrentLevel(LevelPtr level) { 
		level_ = level; 
		loadManager_->SetCurrentLevel(level); }
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
	LevelPtr level_;

	ChunkLoadManager* loadManager_;
};