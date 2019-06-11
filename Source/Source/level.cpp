#include "stdafx.h"
#include "level.h"
#include "pipeline.h"
#include "mesh.h"
#include "texture.h"

#include "transform.h"
#include "mesh_comp.h"
#include "unlit_mesh.h"
#include "render_data.h"

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
	_cameras.push_back(new Camera(kControlCam));
	Render::SetCamera(_cameras[0]);

	GameObjectPtr block = new GameObject();
	block->AddComponent(new Transform());

	RenderDataPtr rend = new RenderData();
	rend->UseUntexturedBlockData();
	block->AddComponent(rend);

	block->SetChildren();
	_objects.push_back(block);
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
