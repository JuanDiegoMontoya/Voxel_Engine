#pragma once
#include "chunk_load_manager.h"
#include "block.h"
#include <set>
#include <unordered_set>
#include "camera.h"
#include "pipeline.h"
#include "chunk.h"

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;
//class ChunkLoadManager;

namespace Utils
{
	struct ChunkPtrKeyEq
	{
		bool operator()(const ChunkPtr& first, const ChunkPtr& second) const
		{

			return
				glm::distance(glm::vec3(first->GetPos() * Chunk::GetChunkSize()), Render::GetCamera()->GetPos()) >
				glm::distance(glm::vec3(second->GetPos() * Chunk::GetChunkSize()), Render::GetCamera()->GetPos());
		}
	};
}

class ChunkManager
{
public:
	ChunkManager();
	~ChunkManager();
	void Init();

	// interaction
	void Update(LevelPtr level);
	void UpdateBlock(glm::ivec3& wpos, Block::BlockType t, unsigned char writeStrength);
	void UpdateBlockCheap(glm::ivec3& wpos, Block block);
	Block GetBlock(glm::ivec3 wpos); // wrapper function
	void UpdatedChunk(ChunkPtr chunk);
	void ReloadAllChunks(); // for when big things change

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

	// generates
	void chunk_generator_thread_task();
	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> generation_queue_;
	std::unordered_set<ChunkPtr> generation_queue_;
	std::mutex chunk_generation_mutex_;
	std::thread* chunk_generator_thread_;
	std::condition_variable generation_ready_;

	// generates meshes for ANY UPDATED chunk
	void chunk_mesher_thread_task();
	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> mesher_queue_;
	std::unordered_set<ChunkPtr> mesher_queue_;
	std::mutex chunk_mesher_mutex_;
	std::thread* chunk_mesher_thread_;
	std::condition_variable mesher_ready_;


	// NOT multithreaded
	void chunk_buffer_task();
	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> buffer_queue_;
	std::unordered_set<ChunkPtr> buffer_queue_;
	std::mutex chunk_buffer_mutex_;


	// vars
	float loadDistance_;
	float unloadLeniency_;
	unsigned maxLoadPerFrame_;
	std::vector<ChunkPtr> updatedChunks_;
	std::vector<ChunkPtr> genChunkList_;
	LevelPtr level_ = nullptr;

	ChunkLoadManager* loadManager_;
};