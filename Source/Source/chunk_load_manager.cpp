#include "stdafx.h"
#include "generation.h"
#include "chunk_load_manager.h"
#include "camera.h"
#include "chunk.h"
#include "pipeline.h"
#include <algorithm>
#include <execution>

void ChunkLoadManager::Update()
{
	cull();
	sort();
}

void ChunkLoadManager::Push(ChunkPtr c)
{
	if (c->NeedsLoading())
	{
		genList_.push_back(c);
	}
}

void ChunkLoadManager::init()
{
	// single extra thread (may increase in the future)
	//pool_.resize(1);
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
	std::remove_if(
		std::execution::par,
		genList_.begin(), 
		genList_.end(),
		[](ChunkPtr c)->bool
	{
		return !c->NeedsLoading();
	});
}

// loading task given to thread pool
void ChunkLoadManager::task(ChunkPtr c)
{
	float dist = glm::distance(glm::vec3(c->GetPos() * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
	if (c)
	{
		// TODO: check if can be loaded from file before attempting to generate
#if MARCHED_CUBES
		WorldGen::Generate3DNoiseChunk(c->GetPos(), level_);
#else
		WorldGen::GenerateChunk(c->GetPos(), level_);
#endif
		c->SetGenerated(false);
		c->SetLoaded(true);
	}
	c->Update();
}

bool ChunkLoadManager::greater(ChunkPtr a, ChunkPtr b, const glm::vec3& camPos)
{
	glm::vec3 ap = a->GetPos() * Chunk::CHUNK_SIZE;
	glm::vec3 bp = b->GetPos() * Chunk::CHUNK_SIZE;
	return glm::distance(ap, camPos) > glm::distance(bp, camPos);
}
