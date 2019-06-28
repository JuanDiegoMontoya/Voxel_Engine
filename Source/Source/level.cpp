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
	for (auto& obj : _objects)
		delete obj;

	for (auto& cam : _cameras)
		delete cam;
}

// for now this function is where we declare objects
void Level::Init()
{
	int f = sizeof(GameObject);
	f = sizeof(RenderData);
	f = sizeof(Shader);
	f = sizeof(std::string);
	
	//std::fill(_blocksarr[0], _blocksarr[100 * 100 * 100], 0);
	std::memset(_blocksarr, 0, sizeof(float) * 150 * 150 * 150);

	_cameras.push_back(new Camera(kControlCam));
	Render::SetCamera(_cameras[0]);

	GameObjectPtr block = new GameObject();
	block->AddComponent(new Transform());
	RenderDataPtr rend = new RenderData();
	rend->UseUntexturedBlockData();
	block->AddComponent(rend);
	block->SetChildren();
	_objects.push_back(block);
	
	// build with /openmp to use this
//#pragma omp parallel for
	for (int i = 0; i < 150; i++)
	{
		for (int j = 0; j < 150; j++)
		{
			for (int k = 0; k < 150; k++)
			{
				//GameObjectPtr two = block->Clone();
				//two->GetComponent<Transform>()->SetTranslation(glm::vec3(i, 1, j));
				float r = Utils::get_random(0, 1);
				float g = Utils::get_random(0, 1);
				float b = Utils::get_random(0, 1);

				//float x = Utils::get_random(-500, 500);
				//float y = Utils::get_random(-500, 500);
				//float z = Utils::get_random(-500, 500);
				//two->GetComponent<RenderData>()->SetColor(glm::vec4(r, g, b, 1.f));
				//_objects.push_back(two);

				BlockPtr block = new Block(_blocks.size());
				block->SetPos(glm::vec3(i, j, k));
				block->clr = glm::vec4(r, g, b, 1);
				_blocks.push_back(block);
				_blocksarr[ID3D(i, j, k, 150, 150)] = block;
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
	for (auto& obj : _objects)
	{
		if (obj && obj->GetEnabled())
		{
			for (size_t i = 0; i < cCount; i++)
			{
				Component* comp = obj->GetComponent(i);
				if (comp && comp->GetEnabled())
					comp->Update(dt);
			}
		}
	}

	//for (auto& block : _blocks)
	//{
	//	glm::ivec3 pos = block->GetPos();
	//	if (pos.y > -500)
	//		block->SetPos(glm::ivec3(pos.x, pos.y - 100, pos.z));
	//}
	//THE_CHOSEN_ONE->SetPos(glm::ivec3(_cameras[0]->GetPos() - 1.f));
	//if ()

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
