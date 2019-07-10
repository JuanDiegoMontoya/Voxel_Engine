#include "stdafx.h"
#include "level.h"
#include "pipeline.h"
#include "mesh.h"
#include "texture.h"

#include "transform.h"
#include "mesh_comp.h"
#include "unlit_mesh.h"
#include "render_data.h"
#include "block.h"
#include "chunk.h"
#include "sun.h"
#include <omp.h>
#include <chrono>
#include <mutex>
#include <execution>
#include "ctpl_stl.h"

static std::mutex mtx;

using namespace std::chrono;
//#define OMP_NUM_THREADS = 8;

Level::Level(std::string name)
{
	name_ = name;
}

Level::~Level()
{
	for (auto& obj : objects_)
		delete obj;

	for (auto& cam : cameras_)
		delete cam;
}

// for now this function is where we declare objects
void Level::Init()
{
	std::memset(Block::blocksarr_, 0, sizeof(float) * 100 * 100 * 100);

	cameras_.push_back(new Camera(kControlCam));
	Render::SetCamera(cameras_[0]);

	int cc = 4; // chunk count
	updatedChunks_.reserve(cc * cc * cc);
	sizeof(Block);
	// initialize a single chunk

	high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

	omp_set_num_threads(8);
	// this loop is not parallelizable (unless VAO constructor is moved)
	for (int xc = 0; xc < cc; xc++)
	{
		for (int yc = 0; yc < cc; yc++)
		{
			for (int zc = 0; zc < cc; zc++)
			{
				Chunk* init = Chunk::chunks[glm::ivec3(xc, yc, zc)] = new Chunk(true);
				init->SetPos(glm::ivec3(xc, yc, zc));
				updatedChunks_.push_back(init);

				for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
				{
					for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
					{
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
						{
							if (Utils::get_random_r(0, 1) > 1.9f)
								continue;
							init->At(x, y, z).SetType(Block::bStone);
						}
					}
				}
			}
		}
	}

/*
#define YEET

	// TODO: chunk loading into memory-managable bites
	// TODO: 64 bit compiling (max blocks WHILE GENERATING ~9 million in 32 bit
	//			 unless broken into smaller chunks at a time)

#ifdef YEET
	int chk = 3;
	for (int i = 0; i < chk; i++)
	{
#endif
#pragma omp parallel for num_threads(8)
#ifdef YEET
		for (int xc = cc / chk * i; xc < cc; xc++)
		{
			if (xc >= cc / chk * (i + 1))
				break;
#else
for (int xc = 0; xc < cc; xc++)
{
#endif
			for (int yc = 0; yc < cc; yc++)
			{
				for (int zc = 0; zc < cc; zc++)
				{
					Chunk::chunks[glm::ivec3(xc, yc, zc)]->Update();
				}
			}
		}

		// this loop cannot be parallelized
#ifdef YEET
		for (int xc = cc / chk * i; xc < cc / chk * (i + 1) && xc < cc; xc++)
#else
		for (int xc = 0; xc < cc; xc++)
#endif
		{
			for (int yc = 0; yc < cc; yc++)
			{
				for (int zc = 0; zc < cc; zc++)
				{
					Chunk::chunks[glm::ivec3(xc, yc, zc)]->BuildBuffers();
				}
			}
		}
#ifdef YEET
	}
#endif
*/

		
	std::for_each(
		std::execution::par_unseq, 
		updatedChunks_.begin(), 
		updatedChunks_.end(),
		[](ChunkPtr& chunk)
	{
		if (chunk)
			chunk->Update();
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
	
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	std::cout << benchmark_duration_.count() << std::endl;
}

// update every object in the level
void Level::Update(float dt)
{
	// update each camera
	for (auto& cam : cameras_)
	{
		cam->Update(dt);
	}

	// render sun first
	sun_.Update();
	sun_.Render();

	// render blocks in each active chunk
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)
			chunk.second->Render();
	});
}

void Level::CheckCollision()
{
}

void Level::CheckInteraction()
{
}
