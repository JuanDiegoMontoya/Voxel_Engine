#include "stdafx.h"
#include "component.h"
#include "level.h"
#include "game_object.h"

GameObject::GameObject()
{
	for (size_t i = 0; i < cCount; i++)
	{
		components_[i] = nullptr;
	}
	// this needs to do some stuff with initializing the object id or something
}

GameObject::~GameObject()
{
	for (int i = 0; i < cCount; i++)
	{
		if (components_[i])
		{
			delete components_[i];
		}
	}
}

GameObjectPtr GameObject::Clone() const
{
	GameObjectPtr newobj = new GameObject();
	for (size_t i = 0; i < cCount; i++)
	{
		if (components_[i])
			newobj->AddComponent(components_[i]->Clone());
	}

	// set all necessary member vars
	newobj->life_ = life_;
	newobj->temporary_ = temporary_;
	newobj->toDestroy_ = toDestroy_;
	newobj->enabled_ = enabled_;
	newobj->name_ = name_;

	return newobj;
}

void GameObject::AddComponent(Component* component)
{
	if (component)
	{
		component->SetParent(this);
		components_[component->GetType()] = component;
	}
}

void GameObject::SetChildren()
{
	for (size_t i = 0; i < cCount; i++)
	{
		if (components_[i])
		{
			components_[i]->SetParent(this);
		}
	}
}
