#pragma once
#include "chunk_load_manager.h"
#include "block.h"
#include <set>
#include <unordered_set>
#include "camera.h"
#include "pipeline.h"
#include "chunk.h"
#include <atomic>
#include <stack>

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;
//class ChunkLoadManager;

namespace Utils
{
	struct ChunkPtrKeyEq
	{
		bool operator()(const ChunkPtr& first, const ChunkPtr& second) const
		{
			//ASSERT(first != second);
			if (first == second)
				return false;
			glm::vec3 wposA = glm::vec3(first->GetPos() * Chunk::GetChunkSize());
			glm::vec3 wposB = glm::vec3(second->GetPos() * Chunk::GetChunkSize());
			glm::vec3 cam = Render::GetCamera()->GetPos();
			return
				glm::distance(wposA, cam) <
				glm::distance(wposB, cam);
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
	void UpdateBlock(glm::ivec3& wpos, Block bl);
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

	friend class Level; // so level can display debug info
private:
	// functions
	void ProcessUpdatedChunks();
	bool isChunkInUpdateList(ChunkPtr chunk);
	void checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near);

	void removeFarChunks();
	void createNearbyChunks(); // and delete far away chunks
	void generateNewChunks();

	// generates
	void chunk_generator_thread_task();
	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> generation_queue_;
	std::unordered_set<ChunkPtr> generation_queue_;
	std::mutex chunk_generation_mutex_;
	std::vector<std::thread*> chunk_generator_threads_;

	// generates meshes for ANY UPDATED chunk
	void chunk_mesher_thread_task();
	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> mesher_queue_;
	//std::set<ChunkPtr> mesher_queue_;
	std::unordered_set<ChunkPtr> mesher_queue_;
	std::mutex chunk_mesher_mutex_;
	std::vector<std::thread*> chunk_mesher_threads_;
	std::atomic_int debug_cur_pool_left = { 0 };


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