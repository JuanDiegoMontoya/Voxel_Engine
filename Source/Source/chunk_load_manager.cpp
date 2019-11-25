#include "stdafx.h"
#include "generation.h"
#include "chunk_load_manager.h"
#include "camera.h"
#include "chunk.h"
#include "pipeline.h"
#include <algorithm>
#include <execution>
#include "level.h"


void ChunkLoadManager::Update()
{
	//cull();
	//sort();
}


void ChunkLoadManager::Push(ChunkPtr c)
{
	//if (c->NeedsLoading())
	{
		//c->SetIsLoading(true); // this may cause problems in the future
		//genList_.push_back(c);
		pool_.push([this](int id, ChunkPtr c) { this->task(id, c); }, c);
	}
}


void ChunkLoadManager::SetCurrentLevel(LevelPtr level)
{
	level_ = level;
}


void ChunkLoadManager::init()
{
	// single thread for this task, as more will likely crash everything
	pool_.resize(1);
}


void ChunkLoadManager::sort()
{
	glm::vec3 camPos = Render::GetCamera()->GetPos();

	// insertion sort chunks in list by distance from camera
	// (to load nearer chunks first)
	for (size_t i = 1; i < genList_.size(); i++)
	{
		ChunkPtr key = genList_[i];
		int j = i - 1;

		while (j >= 0 && greater(genList_[j], key, camPos))
		{
			genList_[j + 1] = genList_[j];
			j--;
		}
		genList_[j + 1] = key;
	}
}


void ChunkLoadManager::cull()
{
	// if the chunk doesn't need to be loaded, remove it from the genList
	std::remove_if(
		std::execution::par,
		genList_.begin(), 
		genList_.end(),
		[](auto& c)->bool
	{
		//return !c->NeedsLoading();
		return true;
	});
}


// loading task given to thread pool
void ChunkLoadManager::task(int id, ChunkPtr c)
{
	if (c)
	{
		// TODO: check if can be loaded from file before attempting to generate
#if MARCHED_CUBES
		WorldGen::Generate3DNoiseChunk(c->GetPos(), level_);
#else
		WorldGen::GenerateChunk(c->GetPos(), level_);
#endif
		//c->SetGenerate(false);
		//c->SetLoaded(true);
		//c->SetIsLoading(false);
		level_->UpdatedChunk(c); // this will cause crashes if loading too many chunks at once
	}
	c->Update();
}


bool ChunkLoadManager::greater(ChunkPtr a, ChunkPtr b, const glm::vec3& camPos)
{
	glm::vec3 ap = a->GetPos() * Chunk::CHUNK_SIZE;
	glm::vec3 bp = b->GetPos() * Chunk::CHUNK_SIZE;
	return glm::distance(ap, camPos) > glm::distance(bp, camPos);
}
