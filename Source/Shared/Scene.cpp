#include "Scene.h"
#include "Entity.h"
#include "Components.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

Entity Scene::CreateEntity(std::string_view name)
{
	Entity entity = { registry_.create(), this };
	entity.AddComponent<TransformComponent>();
	auto& nome = entity.AddComponent<NameComponent>();
	nome.name = name;
}

void Scene::Update(double dt)
{
	//// Render 2D
	//Camera* mainCamera = nullptr;
	//glm::mat4* cameraTransform = nullptr;
	//{
	//	auto group = m_Registry.view<TransformComponent, CameraComponent>();
	//	for (auto entity : group)
	//	{
	//		auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

	//		if (camera.Primary)
	//		{
	//			mainCamera = &camera.Camera;
	//			cameraTransform = &transform.Transform;
	//			break;
	//		}
	//	}
	//}

	//if (mainCamera)
	//{
	//	Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

	//	auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
	//	for (auto entity : group)
	//	{
	//		auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

	//		Renderer2D::DrawQuad(transform, sprite.Color);
	//	}

	//	Renderer2D::EndScene();
	//}
}
