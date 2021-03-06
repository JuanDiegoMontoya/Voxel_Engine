#include "stdafx.h"
#include <algorithm>
#include <execution>
#include <mutex>
#include "chunk.h"
#include "block.h"
#include "World.h"
#include "chunk_manager.h"
#include "generation.h"
#include "pipeline.h"
#include "utilities.h"
#include "ChunkHelpers.h"
#include "ChunkStorage.h"


ChunkManager::ChunkManager()
{
	loadDistance_ = 0;
	unloadLeniency_ = 0;
	debug_cur_pool_left = 0;
}


ChunkManager::~ChunkManager()
{
	shutdownThreads = true;
	for (auto t_ptr : chunk_generator_threads_)
	{
		t_ptr->join();
		delete t_ptr;
	}
	for (auto t_ptr : chunk_mesher_threads_)
	{
		t_ptr->join();
		delete t_ptr;
	}
}


void ChunkManager::Init()
{
	// run main thread on core 1
	//SetThreadAffinityMask(GetCurrentThread(), 1);

	// spawn chunk block generator threads
	//for (int i = 0; i < 4; i++)
	//{
	//	chunk_generator_threads_.push_back(
	//		new std::thread([this]() { chunk_generator_thread_task(); }));
	//	//SetThreadAffinityMask(chunk_generator_threads_[i]->native_handle(), ~1);
	//}

	// spawn chunk mesh generator threads
	for (int i = 0; i < 1; i++)
	{
		chunk_mesher_threads_.push_back(
			new std::thread([this]() { chunk_mesher_thread_task(); }));
		//SetThreadAffinityMask(chunk_mesher_threads_[i]->native_handle(), ~1);
	}
}


void ChunkManager::Update()
{
  PERF_BENCHMARK_START;

	// TODO: update a random selection of the chunks per frame
	//std::for_each(
	//	std::execution::par,
	//	ChunkStorage::GetMapRaw().begin(),
	//	ChunkStorage::GetMapRaw().end(),
	//	[](auto& p)
	//{
	//	if (p.second)
	//		p.second->Update();
	//});

	//chunk_gen_mesh_nobuffer();
	chunk_buffer_task();
	//removeFarChunks();
	//createNearbyChunks();

	for (ChunkPtr chunk : delayed_update_queue_)
		UpdateChunk(chunk);
	delayed_update_queue_.clear();
  PERF_BENCHMARK_END;
}


void ChunkManager::UpdateChunk(ChunkPtr chunk)
{
	ASSERT(chunk != nullptr);
	std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
	mesher_queue_.insert(chunk);
}


void ChunkManager::UpdateChunk(const glm::ivec3 wpos)
{
	auto cpos = ChunkHelpers::worldPosToLocalPos(wpos);
	//auto cptr = Chunk::chunks[cpos.chunk_pos];
	auto cptr = ChunkStorage::GetChunk(cpos.chunk_pos);
	if (cptr)
	{
		std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
		mesher_queue_.insert(cptr);
	}
}


void ChunkManager::UpdateBlock(const glm::ivec3& wpos, Block bl)
{
	ChunkHelpers::localpos p = ChunkHelpers::worldPosToLocalPos(wpos);
	//BlockPtr block = Chunk::AtWorld(wpos);
	Block remBlock = ChunkStorage::AtWorldD(p); // store state of removed block to update lighting
	ChunkPtr chunk = ChunkStorage::GetChunk(p.chunk_pos);

	//if (block)
	//{
	//	// write policy: skip if new block is WEAKER than current block (same strength WILL overwrite)
	//	if (bl.WriteStrength() < block->WriteStrength())
	//		return;
	//}

	// create empty chunk if it's null
	if (!chunk)
	{
		// make chunk, then modify changed block
		ChunkStorage::GetMapRaw()[p.chunk_pos] = chunk = new Chunk();
		chunk->SetPos(p.chunk_pos);
		std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
		generation_queue_.insert(chunk);
		remBlock = chunk->BlockAt(p.block_pos); // remBlock would've been 0 block cuz null, so it's fix here
	}

	chunk->SetBlockTypeAt(p.block_pos, bl.GetType());

	// check if removed block emitted light
	lightPropagateRemove(wpos);

	// check if added block emits light
	glm::uvec3 emit2 = Block::PropertiesTable[int(bl.GetType())].emittance;
	if (emit2 != glm::uvec3(0))
		lightPropagateAdd(wpos, Light(Block::PropertiesTable[int(bl.GetType())].emittance));

	// add to update list if it ain't
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
		// TODO: add 20 more cases for diagonals (AO)
	};
	for (const auto& dir : dirs)
	{
		checkUpdateChunkNearBlock(wpos, dir);
	}

	delayed_update_queue_.erase(chunk);
}


// perform no checks, therefore the chunk must be known prior to placing the block
void ChunkManager::UpdateBlockCheap(const glm::ivec3& wpos, Block block)
{
	auto l = ChunkHelpers::worldPosToLocalPos(wpos);
	ChunkStorage::GetChunk(l.chunk_pos)->SetBlockTypeAt(l.block_pos, block.GetType());
	//*Chunk::AtWorld(wpos) = block;
	//UpdatedChunk(Chunk::chunks[Chunk::worldBlockToLocalPos(wpos).chunk_pos]);
}


void ChunkManager::ReloadAllChunks()
{
	for (const auto& p : ChunkStorage::GetMapRaw())
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
	std::for_each(ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end(), [&](auto& p)
		{
			if (p.second)
				tempChunks.push_back(p.second);
		});
	archive(tempChunks);

	std::cout << "Saved to " << fname << "!\n";
}


void ChunkManager::LoadWorld(std::string fname)
{
	for_each(ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end(), [](auto pair)
	{
		if (pair.second)
			delete pair.second;
	});
	ChunkStorage::GetMapRaw().clear();

	// TODO: fix this (doesn't call serialize functions for some reason)
	std::ifstream is("./resources/Maps/" + fname + ".bin", std::ios::binary);
	cereal::BinaryInputArchive archive(is);
	std::vector<Chunk> tempChunks;
	archive(tempChunks);
	std::for_each(tempChunks.begin(), tempChunks.end(), [&](Chunk& c)
		{
		ChunkStorage::GetMapRaw()[c.GetPos()] = new Chunk(c);
		});

	ReloadAllChunks();
	std::cout << "Loaded " << fname << "!\n";
}


void ChunkManager::checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near)
{
	// skip if both blocks are in same chunk
	auto p1 = ChunkHelpers::worldPosToLocalPos(pos);
	auto p2 = ChunkHelpers::worldPosToLocalPos(pos + near);
	if (p1.chunk_pos == p2.chunk_pos)
		return;

	// update chunk if near block is NOT air/invisible
	//BlockPtr cb = Chunk::AtWorld(pos);
	//BlockPtr nb = Chunk::AtWorld(pos + near);
	//if (cb && nb && nb->GetType() != BlockType::bAir)
	ChunkPtr cptr = ChunkStorage::GetChunk(p2.chunk_pos);
	if (cptr)
	{
		std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
		mesher_queue_.insert(cptr);
		delayed_update_queue_.erase(cptr);
	}
		//if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
		//	updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}


// TODO: make this a Safe & Reliable Operation(tm)
//   rather than having it cause crashes often
void ChunkManager::removeFarChunks()
{
	// delete chunks far from the camera (past leniency range)
	if (generation_queue_.size() == 0 && mesher_queue_.size() == 0 && debug_cur_pool_left == 0)
	{
		std::vector<ChunkPtr> deleteList;
		// attempt at safety
		std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
		std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
		std::lock_guard<std::mutex> lock3(chunk_buffer_mutex_);
		Utils::erase_if(
			ChunkStorage::GetMapRaw(),
			[&](auto& p)->bool
		{
			// range is distance from camera to corner of chunk (corner is ok)
			float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
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
	//	float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
	//	if (p.second)
	//	{
	//		if (dist > loadDistance_ + unloadLeniency_)
	//			p.second->SetActive(false);
	//		else
	//			p.second->SetActive(true);
	//	}
	//});
}


void ChunkManager::createNearbyChunks()
{
	// generate new chunks that are close to the camera
	std::for_each(
		std::execution::par,
		ChunkStorage::GetMapRaw().begin(),
		ChunkStorage::GetMapRaw().end(),
		[&](auto& p)
	{
		float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
		// generate null chunks within distance
		if (!p.second && dist <= loadDistance_)
		{
			p.second = new Chunk();
			p.second->SetPos(p.first);
//			p.second->generate_ = true;
			std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
			generation_queue_.insert(p.second);
		}
	});
}


// TODO: make lighting updates also check chunks around the cell 
// (because lighting affects all neighboring blocks)
// ref https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
// wpos: world position
// nLight: new lighting value
// skipself: chunk updating thing
void ChunkManager::lightPropagateAdd(glm::ivec3 wpos, Light nLight, bool skipself)
{
	// get existing light at the position
	auto optL = ChunkStorage::AtWorldE(wpos);
	if (optL.has_value())
	{
		// if there is already light in the spot,
		// combine the two by taking the max values only
		glm::u8vec4 t = glm::max(optL->GetLight().Get(), nLight.Get());
		ChunkStorage::SetLight(wpos, t);
		//L.Set(t); //*L = t;
	}
	
	// queue of world positions, rather than chunk + local index (they are equivalent)
	std::queue<glm::ivec3> lightQueue;
	lightQueue.push(wpos);

	while (!lightQueue.empty())
	{
		glm::ivec3 lightp = lightQueue.front(); // light position
		lightQueue.pop();
		Light lightLevel = ChunkStorage::AtWorldC(lightp).GetLight(); // node that will be giving light to others
		constexpr glm::ivec3 dirs[] =
		{
			{ 1, 0, 0 },
			{-1, 0, 0 },
			{ 0, 1, 0 },
			{ 0,-1, 0 },
			{ 0, 0, 1 },
			{ 0, 0,-1 },
		};

		// update each neighbor
		for (const auto& dir : dirs)
		{
			glm::ivec3 lightPos = lightp + dir;
			auto block = ChunkStorage::AtWorldE(lightPos);
			if (!block.has_value())
				continue;
			//BlockPtr block = GetBlockPtr(lightp + dir);
			//Block block = GetBlock(lightp + dir); // neighboring block
			//LightPtr light = &block->GetLightRef(); // neighboring light (pointer)
			Light light = block->GetLight();

			// add chunk to update queue if it exists
			if (ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(lightPos).chunk_pos) != nullptr)
				delayed_update_queue_.insert(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(lightPos).chunk_pos));
			
			// invalid light check
			//ASSERT(light != nullptr);

			// if neighbor is solid block, skip dat boi
			if (Block::PropertiesTable[block->GetTypei()].visibility == Visibility::Opaque)
				continue;

			// iterate over R, G, B
			bool enqueue = false;
			for (int ci = 0; ci < 3; ci++)
			{
				// skip blocks that are too bright to be affected by this light
				if (light.Get()[ci] + 2 > lightLevel.Get()[ci])
					continue;

				// TODO: light propagation through transparent materials
				// get all light components (R, G, B, Sun) and modify ONE of them,
				// then push the position of that light into the queue
				//glm::u8vec4 val = light.Get();
				// this line can be optimized to reduce amount of global block getting
				glm::u8vec4 val = ChunkStorage::AtWorldC(lightPos).GetLight().Get();
				val[ci] = (lightLevel.Get()[ci] - 1);// *Block::PropertiesTable[block.GetTypei()].color[ci];
				//light->Set(val);
				ChunkStorage::SetLight(lightPos, val);
				enqueue = true;
			}
			if (enqueue) // enqueue if any lighting component changed
				lightQueue.push(lightPos);
		}
	}

	// do not update this chunk again if it contained the placed light
	if (skipself)
		delayed_update_queue_.erase(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
}


void ChunkManager::lightPropagateRemove(glm::ivec3 wpos)
{
	std::queue<std::pair<glm::ivec3, Light>> lightRemovalQueue;
	Light light = ChunkStorage::AtWorldC(wpos).GetLight();
	lightRemovalQueue.push({ wpos, light });
	ChunkStorage::SetLight(wpos, Light({ 0, 0, 0, light.GetS() }));
	//GetBlockPtr(wpos)->GetLightRef().Set({ 0, 0, 0, light.GetS() });

	std::queue<std::pair<glm::ivec3, Light>> lightReadditionQueue;

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

		for (const auto& dir : dirs)
		{
			glm::ivec3 blockPos = plight + dir;
			auto optB = ChunkStorage::AtWorldE(blockPos);
			//BlockPtr b = GetBlockPtr(plight + dir);
			if (!optB.has_value())
				continue;
			for (int ci = 0; ci < 3; ci++) // iterate 3 color components (not sunlight)
			{

				// if the removed block emits light, it needs to be re-propagated
				auto emit = Block::PropertiesTable[optB->GetTypei()].emittance;
				if (emit != glm::u8vec4(0))
					lightReadditionQueue.push({ blockPos, emit });
				Light nearLight = optB->GetLight();
				//Light& nearLight = b->GetLightRef();
				glm::u8vec4 nlightv = nearLight.Get(); // near light value

				// skip updates when light is 0
				// remove light if there is any and if it is weaker than this node's light value
				//if (nlightv[ci] != 0)
				{
					if (nlightv[ci] != 0 && nlightv[ci] == lightv[ci] - 1)
					{
						lightRemovalQueue.push({ blockPos, nearLight });
						if (ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos))
							delayed_update_queue_.insert(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos));
						auto tmp = nearLight.Get();
						tmp[ci] = 0;
						//nearLight.Set(tmp);
						optB->GetLightRef().Set(tmp);
						ChunkStorage::SetLight(blockPos, tmp);
					}
					// re-propagate near light that is equal to or brighter than this after setting it all to 0
					else if (nlightv[ci] > lightv[ci])
					{
						glm::u8vec4 nue(0);
						nue[ci] = nlightv[ci];
						lightReadditionQueue.push({ blockPos, nue });
					}
				}
			}
		}
	}


	// re-propogate lights in queue, otherwise we're left with hard edges
	while (!lightReadditionQueue.empty())
	{
		const auto& p = lightReadditionQueue.front();
		lightReadditionQueue.pop();
		lightPropagateAdd(p.first, p.second, false);
	}

	// do not update the removed block's chunk again since the act of removing will update it
	delayed_update_queue_.erase(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
}


bool ChunkManager::checkDirectSunlight(glm::ivec3 wpos)
{
	auto p = ChunkHelpers::worldPosToLocalPos(wpos);
	ChunkPtr chunk = ChunkStorage::GetChunk(p.chunk_pos);
	if (!chunk)
		return false;
	Block block = chunk->BlockAt(p.block_pos);

	// find the highest valid chunk
	constexpr glm::ivec3 up(0, 1, 0);
	glm::ivec3 cpos = p.chunk_pos + up;
	ChunkPtr next = chunk;
	while (next)
	{
		chunk = next;
		cpos += up;
		next = ChunkStorage::GetChunk(cpos);
	}

	// go down until we hit another solid block or this block
	return false;
}


void ChunkManager::sunlightPropagateAdd(glm::ivec3 wpos, uint8_t intensity)
{

}


void ChunkManager::sunlightPropagateRemove(glm::ivec3 wpos)
{

}