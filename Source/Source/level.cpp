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
#include <omp.h>
#include <chrono>
#include <mutex>

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

	int cc = 6; // chunk count
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

				for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
				{
					for (int y = 0; y < Chunk::CHUNK_SIZE; y++)
					{
						for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
						{
							if (Utils::get_random_r(0, 1) > .9f)
								continue;
							init->At(x, y, z).SetType(Block::bStone);
						}
					}
				}
			}
		}
	}

	// TODO: chunk loading into memory-managable bites
	// TODO: 64 bit compiling (max blocks WHILE GENERATING ~9 million in 32 bit
	//			 unless broken into smaller chunks at a time)
#pragma omp parallel for
	for (int xc = 0; xc < cc; xc++)
	{
		for (int yc = 0; yc < cc; yc++)
		{
			for (int zc = 0; zc < cc; zc++)
			{
				Chunk::chunks[glm::ivec3(xc, yc, zc)]->Update();
			}
		}
	}

	// this loop cannot be parallelized
	for (int xc = 0; xc < cc; xc++)
	{
		for (int yc = 0; yc < cc; yc++)
		{
			for (int zc = 0; zc < cc; zc++)
			{
				Chunk::chunks[glm::ivec3(xc, yc, zc)]->BuildBuffers();
			}
		}
	}

	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
	std::cout << benchmark_duration_.count() * 1000 << std::endl;
}

// update every object in the level
void Level::Update(float dt)
{
	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
		[](std::pair<glm::ivec3, Chunk*> chunk)
	{
		if (chunk.second)
			chunk.second->Render();
	});

	for (auto& cam : cameras_)
	{
		cam->Update(dt);
	}
}

void Level::CheckCollision()
{
}

void Level::CheckInteraction()
{
}
