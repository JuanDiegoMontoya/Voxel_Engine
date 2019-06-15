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

	_cameras.push_back(new Camera(kControlCam));
	Render::SetCamera(_cameras[0]);

	GameObjectPtr block = new GameObject();
	block->AddComponent(new Transform());
	RenderDataPtr rend = new RenderData();
	rend->UseUntexturedBlockData();
	block->AddComponent(rend);
	block->SetChildren();
	_objects.push_back(block);
	
	for (int i = 0; i < 1000; i++)
	{
		for (int j = 0; j < 1000; j++)
		{
			//GameObjectPtr two = block->Clone();
			//two->GetComponent<Transform>()->SetTranslation(glm::vec3(i, 1, j));
			float r = Utils::get_random(0, 1);
			float g = Utils::get_random(0, 1);
			float b = Utils::get_random(0, 1);
			//two->GetComponent<RenderData>()->SetColor(glm::vec4(r, g, b, 1.f));
			//_objects.push_back(two);

			BlockPtr block = new Block(_blocks.size());
			block->SetPos(glm::vec3(i, 1, j));
			block->clr = glm::vec4(r, g, b, 1);
			_blocks.push_back(block);
		}
	}
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
