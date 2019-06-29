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

Level::Level(std::string name)
{
	_name = name;
}

Level::~Level()
{
	for (auto& cam : _cameras)
		delete cam;
}

// for now this function is where we declare objects
void Level::Init()
{
	_cameras.push_back(new Camera(kControlCam));
	Render::SetCamera(_cameras[0]);
	
	// build with /openmp to use this
//#pragma omp parallel for
	for (int i = 0; i < 150; i++)
	{
		for (int j = 0; j < 150; j++)
		{
			for (int k = 0; k < 150; k++)
			{
				k = sizeof(Block);
			}
		}
	}
	//std::cout << _blocks.size();

	//CHOSEN_POS = ID3D(0, 0, 0, 100, 100);
	//THE_CHOSEN_ONE = _blocksarr[CHOSEN_POS];
}

// update every object in the level
void Level::Update(float dt)
{
	for (auto& cam : _cameras)
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
