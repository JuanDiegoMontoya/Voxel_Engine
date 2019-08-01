#include "stdafx.h"
#include "chunk.h"
#include "level.h"
#include "chunk_manager.h"
#include "generation.h"
#include "pipeline.h"

#include <algorithm>
#include <execution>

ChunkManager::ChunkManager()
{
	Chunk::chunks.max_load_factor(0.7f);
	loadDistance_ = 0;
	unloadLeniency_ = 0;
	maxLoadPerFrame_ = 0;
}

void ChunkManager::Update(LevelPtr level)
{
	ProcessUpdatedChunks();
	createNearbyChunks();
	generateNewChunks();
}

void ChunkManager::UpdateBlock(glm::ivec3& wpos, Block::BlockType t)
{
	localpos p = Chunk::worldBlockToLocalPos(wpos);
	BlockPtr block = Chunk::AtWorld(wpos);
	ChunkPtr chunk = Chunk::chunks[p.chunk_pos];

	if (block && block->GetType() == t) // ignore if same type
		return;

	// create empty chunk if it's null
	if (!chunk)
	{
		Chunk::chunks[p.chunk_pos] = chunk = new Chunk(true);
		chunk->SetPos(p.chunk_pos);
	}

	if (!block) // skip null blocks
		block = &chunk->At(p.block_pos);
	block->SetType(t);

	// add to update list if it ain't
	if (!isChunkInUpdateList(chunk))
		updatedChunks_.push_back(chunk);

	// check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
	glm::ivec3 dirs[] =
	{
		{ -1,  0,  0 },
		{  1,  0,  0 },
		{  0, -1,  0 },
		{  0,  1,  0 },
		{  0,  0, -1 },
		{  0,  0,  1 }
	};
	for (const auto& dir : dirs)
	{
		checkUpdateChunkNearBlock(wpos, dir);
	}
}

void ChunkManager::ProcessUpdatedChunks()
{
	std::for_each(
		std::execution::par_unseq,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		if (chunk)
			chunk->BuildMesh();
	});

	// this operation cannot be parallelized
	std::for_each(
		std::execution::seq,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		if (chunk)
			chunk->BuildBuffers();
	});

	updatedChunks_.clear();
}

bool ChunkManager::isChunkInUpdateList(ChunkPtr c)
{
	for (auto& chunk : updatedChunks_)
	{
		if (chunk == c)
			return true;
	}
	return false;
}

void ChunkManager::checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near)
{
	localpos p1 = Chunk::worldBlockToLocalPos(pos);
	localpos p2 = Chunk::worldBlockToLocalPos(pos + near);
	if (p1.chunk_pos == p2.chunk_pos)
		return;

	// update chunk if near block is NOT air/invisible
	BlockPtr cb = Chunk::AtWorld(pos);
	BlockPtr nb = Chunk::AtWorld(pos + near);
	if (cb && nb && nb->GetType() != Block::bAir)
		if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
			updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}

void ChunkManager::createNearbyChunks()
{
	// delete far away chunks, then create chunks that are close
	std::for_each(
		std::execution::seq,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[&](auto& p)
	{
		float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
		if (p.second)
		{
			if (dist > loadDistance_ + unloadLeniency_)
			{
				delete p.second;
				p.second = nullptr;
				return;
			}
		}
		else if (dist <= loadDistance_)
		{
			p.second = new Chunk(true);
			p.second->SetPos(p.first);
			p.second->generate_ = true;
		}
	});
}

void ChunkManager::generateNewChunks()
{
	//std::for_each(
//	Chunk::chunks.begin(),
//	Chunk::chunks.end(),
//	[&](auto& p)
//{
//	if (p.second)
//		if (p.second->generate_)
//			genChunks.push_back(p.second);
//});

//// sort chunks to update by distance from camera
//std::sort(std::execution::par,
//	genChunks.begin(),
//	genChunks.end(),
//	[&](ChunkPtr& a, ChunkPtr& b)->bool
//{
//	return glm::distance(Render::GetCamera()->GetPos(), glm::vec3(a->GetPos() * Chunk::CHUNK_SIZE)) <
//				 glm::distance(Render::GetCamera()->GetPos(), glm::vec3(b->GetPos() * Chunk::CHUNK_SIZE));
//});

// generate each chunk that needs to be
	unsigned maxgen = maxLoadPerFrame_;
	std::for_each(
		std::execution::seq,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[&](auto& p)
	{
		ChunkPtr chunk = p.second;
		if (chunk)
		{
			if (chunk->generate_ && maxgen > 0)
			{
#if MARCHED_CUBES
				WorldGen::Generate3DNoiseChunk(chunk->GetPos(), level_);
#else
				WorldGen::GenerateChunk(chunk->GetPos(), level_);
#endif
				chunk->generate_ = false;
				maxgen--;
			}
			chunk->Update();
		}
	});

	//genChunks.clear();
}
