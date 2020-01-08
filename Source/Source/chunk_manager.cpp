#include "stdafx.h"
#include <algorithm>
#include <execution>
#include <mutex>
#include "chunk_load_manager.h"
#include "chunk.h"
#include "block.h"
#include "level.h"
#include "chunk_manager.h"
#include "generation.h"
#include "pipeline.h"
#include "utilities.h"

// Windows-specific thread affinity
#ifdef _WIN32
#include <Windows.h>
#undef near
#endif


ChunkManager::ChunkManager()
{
	Chunk::chunks.max_load_factor(0.7f);
	loadDistance_ = 0;
	unloadLeniency_ = 0;
	maxLoadPerFrame_ = 0;
	loadManager_ = new ChunkLoadManager();
	debug_cur_pool_left = 0;
}


ChunkManager::~ChunkManager()
{
	for (auto t_ptr : chunk_generator_threads_)
		delete t_ptr;
	for (auto t_ptr : chunk_mesher_threads_)
		delete t_ptr;
}


void ChunkManager::Init()
{
	// run main thread on core 1
	SetThreadAffinityMask(GetCurrentThread(), 1);

	// spawn chunk block generator threads
	for (int i = 0; i < 4; i++)
	{
		chunk_generator_threads_.push_back(
			new std::thread([this]() { chunk_generator_thread_task(); }));
		SetThreadAffinityMask(chunk_generator_threads_[i]->native_handle(), ~1);
	}

	// spawn chunk mesh generator threads
	for (int i = 0; i < 1; i++)
	{
		chunk_mesher_threads_.push_back(
			new std::thread([this]() { chunk_mesher_thread_task(); }));
		SetThreadAffinityMask(chunk_mesher_threads_[i]->native_handle(), ~1);
	}
}


void ChunkManager::Update(LevelPtr level)
{
  PERF_BENCHMARK_START;
	std::for_each(
		std::execution::par,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[](auto& p)
	{
		if (p.second)
			p.second->Update();
	});

	//ProcessUpdatedChunks();
	chunk_buffer_task();
	removeFarChunks();
	createNearbyChunks();
	//generateNewChunks();
  PERF_BENCHMARK_END;
}


void ChunkManager::UpdateChunk(const glm::ivec3 wpos)
{
	auto cpos = Chunk::worldBlockToLocalPos(wpos);
	auto cptr = Chunk::chunks[cpos.chunk_pos];
	if (cptr)
	{
		std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
		mesher_queue_.insert(cptr);
	}
}


void ChunkManager::UpdateBlock(const glm::ivec3& wpos, Block bl)
{
	localpos p = Chunk::worldBlockToLocalPos(wpos);
	BlockPtr block = Chunk::AtWorld(wpos);
	Block remBlock = block ? *block : Block(); // store state of removed block to update lighting
	ChunkPtr chunk = Chunk::chunks[p.chunk_pos];

	if (block)
	{
		// write policy: skip if new block is WEAKER than current block (same strength WILL overwrite)
		if (bl.WriteStrength() < block->WriteStrength())
			return;
	}

	// create empty chunk if it's null
	if (!chunk)
	{
		Chunk::chunks[p.chunk_pos] = chunk = new Chunk(true);
		chunk->SetPos(p.chunk_pos);
		std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
		generation_queue_.insert(chunk);
		//chunk->generate_ = true;
	}

	if (!block) // reset block if it's invalid
		block = &chunk->At(p.block_pos);
	block->SetType(bl.GetType(), bl.WriteStrength());

	// check if removed block emitted light
	//glm::uvec3 emit1 = Block::PropertiesTable[int(remBlock.GetType())].emittance;
	//auto emit1 = Chunk::LightAtWorld(wpos)->Get();
	//if (emit1 != glm::uvec3(0))
	//if (Chunk::LightAtWorld(wpos)->Raw() != 0)
		lightPropagateRemove(wpos);

	// check if added block emits light
	glm::uvec3 emit2 = Block::PropertiesTable[int(bl.GetType())].emittance;
	if (emit2 != glm::uvec3(0))
		lightPropagateAdd(wpos, Light(Block::PropertiesTable[int(bl.GetType())].emittance));

	// add to update list if it ain't
	//if (!isChunkInUpdateList(chunk))
	//	updatedChunks_.push_back(chunk);
	{ // scoped, otherwise deadlock will occur in 'checkUpdateChunkNearBlock'
		std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
		mesher_queue_.insert(chunk);
	}

	// check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
	constexpr glm::ivec3 dirs[] =
	{
		{-1, 0, 0 },
		{ 1, 0, 0 },
		{ 0,-1, 0 },
		{ 0, 1, 0 },
		{ 0, 0,-1 },
		{ 0, 0, 1 }
		// TODO: add 8 more cases for diagonals (AO)
	};
	for (const auto& dir : dirs)
	{
		checkUpdateChunkNearBlock(wpos, dir);
	}

}


// perform no checks, therefore the chunk must be known prior to placing the block
void ChunkManager::UpdateBlockCheap(const glm::ivec3& wpos, Block block)
{
	*Chunk::AtWorld(wpos) = block;
	//UpdatedChunk(Chunk::chunks[Chunk::worldBlockToLocalPos(wpos).chunk_pos]);
}


void ChunkManager::UpdateBlockLight(const glm::ivec3 wpos, const Light light)
{
	Block block = GetBlock(wpos);
	//block.SetLightValue(light.r);
	//UpdateBlock(wpos, block);
}


Block ChunkManager::GetBlock(const glm::ivec3 wpos)
{
	BlockPtr block = Chunk::AtWorld(wpos);
	if (!block)
		return Block();
	return *block;
}


Light ChunkManager::GetLight(const glm::ivec3 wpos)
{
	LightPtr light = Chunk::LightAtWorld(wpos);
	if (!light)
		return Light();
	return *light;
}


BlockPtr ChunkManager::GetBlockPtr(const glm::ivec3 wpos)
{
	return Chunk::AtWorld(wpos);
}


LightPtr ChunkManager::GetLightPtr(const glm::ivec3 wpos)
{
	return Chunk::LightAtWorld(wpos);
}


void ChunkManager::UpdatedChunk(ChunkPtr chunk)
{
	//if (isChunkInUpdateList(chunk))
	//	updatedChunks_.push_back(chunk);
	//std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
	std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
	mesher_queue_.insert(chunk);
}


void ChunkManager::ReloadAllChunks()
{
	for (const auto& p : Chunk::chunks)
	{
		if (p.second)
		{
			//std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
			std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
			mesher_queue_.insert(p.second);
		}
			//if (!isChunkInUpdateList(p.second))
			//	updatedChunks_.push_back(p.second);
	}
}


#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/unordered_map.hpp>
#include <fstream>
void ChunkManager::SaveWorld(std::string fname)
{
	std::ofstream of("./resources/Maps/" + fname + ".bin", std::ios::binary);
	cereal::BinaryOutputArchive archive(of);
	std::vector<Chunk*> tempChunks;
	//tempChunks.insert(tempChunks.begin(), Chunk::chunks.begin(), Chunk::chunks.end());
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(), [&](auto& p)
		{
			if (p.second)
				tempChunks.push_back(p.second);
		});
	archive(tempChunks);

	std::cout << "Saved to " << fname << "!\n";
}


void ChunkManager::LoadWorld(std::string fname)
{
	for_each(Chunk::chunks.begin(), Chunk::chunks.end(), [](auto pair)
	{
		if (pair.second)
			delete pair.second;
	});
	Chunk::chunks.clear();

	// TODO: fix this (doesn't call serialize functions for some reason)
	std::ifstream is("./resources/Maps/" + fname + ".bin", std::ios::binary);
	cereal::BinaryInputArchive archive(is);
	std::vector<Chunk> tempChunks;
	archive(tempChunks);
	std::for_each(tempChunks.begin(), tempChunks.end(), [&](Chunk& c)
		{
			Chunk::chunks[c.GetPos()] = new Chunk(c);
		});

	std::cout << "Loaded " << fname << "!\n";
}


// theoretically deprecated
void ChunkManager::ProcessUpdatedChunks()
{
	std::for_each(
		std::execution::par,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		//if (chunk && !chunk->NeedsLoading())
			chunk->BuildMesh();
	});

	// this operation cannot be parallelized
	std::for_each(
		std::execution::seq,
		updatedChunks_.begin(),
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		//if (chunk && !chunk->NeedsLoading())
			chunk->BuildBuffers();
	});

	updatedChunks_.clear();
}


// theoretically deprecated
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
	// skip if both blocks are in same chunk
	localpos p1 = Chunk::worldBlockToLocalPos(pos);
	localpos p2 = Chunk::worldBlockToLocalPos(pos + near);
	if (p1.chunk_pos == p2.chunk_pos)
		return;

	// update chunk if near block is NOT air/invisible
	BlockPtr cb = Chunk::AtWorld(pos);
	BlockPtr nb = Chunk::AtWorld(pos + near);
	if (cb && nb && nb->GetType() != BlockType::bAir)
	{
		std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
		mesher_queue_.insert(Chunk::chunks[p2.chunk_pos]);
	}
		//if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
		//	updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}


void ChunkManager::removeFarChunks()
{
	// delete chunks far from the camera (past leniency range)
	if (generation_queue_.size() == 0 && mesher_queue_.size() == 0 && debug_cur_pool_left.load() == 0)
	{
		std::vector<ChunkPtr> deleteList;
		Utils::erase_if(
			Chunk::chunks,
			[&](auto& p)->bool
		{
			float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
			if (p.second && dist > loadDistance_ + unloadLeniency_)
			{
				deleteList.push_back(p.second);
				return true;
			}
			return false;
		});

		for (ChunkPtr p : deleteList)
			delete p;
	}

	//std::for_each(
	//	std::execution::par,
	//	Chunk::chunks.begin(),
	//	Chunk::chunks.end(),
	//	[&](auto& p)
	//{
	//	float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
	//	if (p.second)
	//	{
	//		if (dist > loadDistance_ + unloadLeniency_)
	//			p.second->SetActive(false);
	//		else
	//			p.second->SetActive(true);
	//	}
	//});
}


void ChunkManager::createNearbyChunks() // and delete ones outside of leniency distance
{
	// generate new chunks that are close to the camera
	std::for_each(
		std::execution::par,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[&](auto& p)
	{
		float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
		// generate null chunks within distance
		if (!p.second && dist <= loadDistance_)
		{
			p.second = new Chunk(true);
			p.second->SetPos(p.first);
//			p.second->generate_ = true;
			std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
			generation_queue_.insert(p.second);
		}
	});
}


void ChunkManager::generateNewChunks()
{
//	std::for_each(
//	Chunk::chunks.begin(),
//	Chunk::chunks.end(),
//	[&](auto& p)
//{
//	float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
//	if (p.second && dist <= loadDistance_)
//		if (p.second->generate_)
//			genChunkList_.push_back(p.second);
//});
//
//	// sort chunks to update by distance from camera
//	std::sort(
//	std::execution::par,
//	genChunkList_.begin(),
//	genChunkList_.end(),
//	[&](ChunkPtr& a, ChunkPtr& b)->bool
//{
//	return glm::distance(Render::GetCamera()->GetPos(), glm::vec3(a->GetPos() * Chunk::CHUNK_SIZE)) <
//				 glm::distance(Render::GetCamera()->GetPos(), glm::vec3(b->GetPos() * Chunk::CHUNK_SIZE));
//});

	//genChunkList_.erase(genChunkList_.begin(), genChunkList_.begin() + maxLoadPerFrame_);
	//genChunkList_.clear();

#if USE_MULTITHREADED_CHUNK_LOADER
	// version 2.0
	for (auto& p : Chunk::chunks)
	{
		float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
		if (p.second && !p.second->InLoadQueue() && p.second->NeedsLoading())
			if (dist <= loadDistance_)
				loadManager_->Push(p.second);
	}
#else
	// generate each chunk that needs to be generated
	unsigned maxgen = maxLoadPerFrame_;
	std::for_each(
		std::execution::seq,
		Chunk::chunks.begin(),
		Chunk::chunks.end(),
		[&](auto& p)
	{
		float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Render::GetCamera()->GetPos());
		ChunkPtr chunk = p.second;
		if (chunk && maxgen > 0 && dist <= loadDistance_)
		{
//			if (chunk->generate_)
			{
#if MARCHED_CUBES
				WorldGen::Generate3DNoiseChunk(glm::vec3(chunk->GetPos()), level_);
#else
				WorldGen::GenerateChunk(chunk->GetPos(), level_);
#endif
//				chunk->SetGenerate(false);
//				chunk->SetLoaded(true);
				//chunk->SetIsLoading(false);
				maxgen--;
			}
			chunk->Update();
		}
			});
#endif
}


// perpetual thread task to generate blocks in new chunks
void ChunkManager::chunk_generator_thread_task()
{
	while (1)
	{
		//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> temp;
		std::unordered_set<ChunkPtr> temp;
		{
			std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
			temp.swap(generation_queue_);
		}

		std::for_each(std::execution::seq, temp.begin(), temp.end(), [this](ChunkPtr chunk)
		{
			WorldGen::GenerateChunk(chunk->GetPos(), level_);
			std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
			mesher_queue_.insert(chunk);
		});

		//std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
		//mesher_queue_.insert(temp.begin(), temp.end());
	}
}


// perpetual thread task to generate meshes for updated chunks
void ChunkManager::chunk_mesher_thread_task()
{
	while (1)
	{
		//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> temp;
		//std::set<ChunkPtr> temp;
		std::unordered_set<ChunkPtr> temp;
		std::vector<ChunkPtr> yeet; // to-be ordered set containing temp's items
		{
			std::lock_guard<std::mutex> lock1(chunk_mesher_mutex_);
			temp.swap(mesher_queue_);
			debug_cur_pool_left += temp.size();
		}
		yeet.insert(yeet.begin(), temp.begin(), temp.end());

		// TODO: this is temp solution to load near chunks to camera first
		std::sort(yeet.begin(), yeet.end(), Utils::ChunkPtrKeyEq());
		std::for_each(std::execution::seq, yeet.begin(), yeet.end(), [this](ChunkPtr chunk)
		{
			//SetThreadAffinityMask(GetCurrentThread(), ~1);
			// send each mesh to GPU immediately after building it
			chunk->BuildMesh();
			debug_cur_pool_left--;
			std::lock_guard<std::mutex> lock2(chunk_buffer_mutex_);
			buffer_queue_.insert(chunk);
		});
	}
	//std::shared_ptr<void> fdsa;
	//fdsa.use_count();
}


// sends vertex data of fully-updated chunks to GPU from main thread (fast and simple)
void ChunkManager::chunk_buffer_task()
{
	//{
	//	std::unordered_set<ChunkPtr> temp;
	//	{
	//		std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
	//		temp.swap(generation_queue_);
	//	}

	//	std::for_each(std::execution::seq, temp.begin(), temp.end(), [this](ChunkPtr chunk)
	//		{
	//			WorldGen::GenerateChunk(chunk->GetPos(), level_);
	//			std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
	//			mesher_queue_.insert(chunk);
	//		});
	//}
	//{
	//	std::unordered_set<ChunkPtr> temp;
	//	std::vector<ChunkPtr> yeet; // to-be ordered set containing temp's items
	//	{
	//		std::lock_guard<std::mutex> lock1(chunk_mesher_mutex_);
	//		temp.swap(mesher_queue_);
	//		debug_cur_pool_left += temp.size();
	//	}
	//	yeet.insert(yeet.begin(), temp.begin(), temp.end());

	//	// TODO: this is temp solution to load near chunks to camera first
	//	std::sort(yeet.begin(), yeet.end(), Utils::ChunkPtrKeyEq());
	//	std::for_each(std::execution::seq, yeet.begin(), yeet.end(), [this](ChunkPtr chunk)
	//		{
	//			//SetThreadAffinityMask(GetCurrentThread(), ~1);
	//			// send each mesh to GPU immediately after building it
	//			chunk->BuildMesh();
	//			debug_cur_pool_left--;
	//			std::lock_guard<std::mutex> lock2(chunk_buffer_mutex_);
	//			buffer_queue_.insert(chunk);
	//		});
	//}







	//std::set<ChunkPtr, Utils::ChunkPtrKeyEq> temp;
	std::unordered_set<ChunkPtr> temp;
	{
		std::lock_guard<std::mutex> lock(chunk_buffer_mutex_);
		temp.swap(buffer_queue_);
	}

	// normally, there will only be a few items in here per frame
	for (ChunkPtr chunk : temp)
		chunk->BuildBuffers();
}


// ref https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
void ChunkManager::lightPropagateAdd(glm::ivec3 wpos, Light nLight)
{
	//UpdateBlockLight(wpos, Block::PropertiesTable[bt].emittance);
	LightPtr L = GetLightPtr(wpos);
	if (L)
		*L = nLight;
	std::queue<glm::ivec3> lightQueue;
	lightQueue.push(wpos);
	while (!lightQueue.empty())
	{
		glm::ivec3 lightp = lightQueue.front();
		lightQueue.pop();
		Light lightLevel = GetLight(lightp);
		constexpr glm::ivec3 dirs[] =
		{
			{ 1, 0, 0 },
			{-1, 0, 0 },
			{ 0, 1, 0 },
			{ 0,-1, 0 },
			{ 0, 0, 1 },
			{ 0, 0,-1 },
		};

		for (const auto& dir : dirs)
		{
			Block block = GetBlock(lightp + dir);
			LightPtr light = GetLightPtr(lightp + dir);
			//UpdateChunk(lightp + dir);
			if (!light) continue;
			// if solid block or too bright of a block, skip dat boi
			if (Block::PropertiesTable[block.GetTypei()].color.a == 1)
				continue;
			// iterate for R, G, B, and sunlight
			for (int ci = 0; ci < 4; ci++) // iterate color index
			{
				// skip blocks that are too bright to be affected by this light
				if (light->Get()[ci] + 2 > lightLevel.Get()[ci])
					continue;
				//UpdateBlockLight(lightp + dir, glm::uvec3(lightLevel.r - 1));
				auto val = light->Get();
				val[ci] = (lightLevel.Get()[ci] - 1) * Block::PropertiesTable[block.GetTypei()].color[ci];
				light->Set(val);
				lightQueue.push(lightp + dir);
			}
		}
	}
}


void ChunkManager::lightPropagateRemove(glm::ivec3 wpos)
{
	std::queue<std::pair<glm::ivec3, Light>> lightRemovalQueue;
	Light light = GetLight(wpos);
	lightRemovalQueue.push({ wpos, light });
	GetLightPtr(wpos)->Set({ 0, 0, 0, light.GetS() });

	std::queue<std::pair<glm::ivec3, Light>> lightReadditionQueue;
	std::unordered_map<glm::ivec3, Light, Utils::ivec3Hash, Utils::ivec3KeyEq> lightsToReadd;

	//return;

	while (!lightRemovalQueue.empty())
	{
		auto [ plight, lite ] = lightRemovalQueue.front();
		auto lightv = lite.Get(); // current light value
		lightRemovalQueue.pop();

		constexpr glm::ivec3 dirs[] =
		{
			{ 1, 0, 0 },
			{-1, 0, 0 },
			{ 0, 1, 0 },
			{ 0,-1, 0 },
			{ 0, 0, 1 },
			{ 0, 0,-1 },
		};

		for (int ci = 0; ci < 3; ci++) // iterate 3 color components (not sunlight)
		{
			//if (lightv[ci] == 0)
			//	continue;
			for (const auto& dir : dirs)
			{
				LightPtr nearLight = GetLightPtr(plight + dir);
				if (!nearLight)
					continue;
				glm::ucvec4 nlightv = nearLight->Get(); // near light value

				// skip updates when light is 0
				// remove light if there is any and if it is weaker than this node's light value
				//if (nlightv[ci] != 0)
				{
					if (nlightv[ci] != 0 && nlightv[ci] == lightv[ci] - 1)
					{
						lightRemovalQueue.push({ plight + dir, *nearLight });
						//UpdateChunk(plight + dir);
						auto tmp = nearLight->Get();
						tmp[ci] = 0;
						nearLight->Set(tmp);
					}
					// re-propagate near light that is equal to or brighter than this after setting it all to 0
					else if (nlightv[ci] > lightv[ci])
					{
						glm::ucvec4 nue(0);
						nue[ci] = nlightv[ci];
						//lightReadditionQueue.push({ plight + dir, nue });
						if (!lightsToReadd[plight + dir].Get()[ci])
						{
							nue += lightsToReadd[plight + dir].Get();
							lightsToReadd[plight + dir].Set(nue);
						}
					}
				}
			}
		}
	}

	// commence re-propogation of light unrelated to the deleted one
	//while (!lightReadditionQueue.empty())
	//{
	//	const auto& p = lightReadditionQueue.front();
	//	lightReadditionQueue.pop();
	//	lightPropagateAdd(p.first, p.second);
	//}

	for (const auto& p : lightsToReadd)
		lightPropagateAdd(p.first, p.second);
}