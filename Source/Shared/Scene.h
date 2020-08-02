#pragma once

#include <entt.hpp>
#include <camera.h>

class Entity;

class Scene
{
public:

	Scene();
	~Scene();

	Entity CreateEntity(std::string_view name);
	void Update(double dt);

	entt::registry& GetRegistry() { return registry_; }

private:
	friend class Entity;
	entt::registry registry_;
	Camera* mainCamera = nullptr;
};