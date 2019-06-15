#include "stdafx.h"
#include "component.h"
#include "level.h"
#include "game_object.h"

GameObject::GameObject()
{
	for (size_t i = 0; i < cCount; i++)
	{
		_components[i] = nullptr;
	}
	// this needs to do some stuff with initializing the object id or something
}

GameObject::~GameObject()
{
	for (int i = 0; i < cCount; i++)
	{
		if (_components[i])
		{
			delete _components[i];
		}
	}
}

GameObjectPtr GameObject::Clone() const
{
	GameObjectPtr newobj = new GameObject();
	for (size_t i = 0; i < cCount; i++)
	{
		if (_components[i])
			newobj->AddComponent(_components[i]->Clone());
	}

	// set all necessary member vars
	newobj->_life = _life;
	newobj->_temporary = _temporary;
	newobj->_toDestroy = _toDestroy;
	newobj->_enabled = _enabled;
	newobj->_name = _name;

	return newobj;
}

void GameObject::AddComponent(Component* component)
{
	if (component)
	{
		component->SetParent(this);
		_components[component->GetType()] = component;
	}
}

void GameObject::SetChildren()
{
	for (size_t i = 0; i < cCount; i++)
	{
		if (_components[i])
		{
			_components[i]->SetParent(this);
		}
	}
}
