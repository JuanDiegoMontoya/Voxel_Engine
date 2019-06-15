#pragma once
#include "component.h"

typedef class GameObject
{
public:
	template<typename T>
	inline T* GetComponent(ComponentType typeId)
	{
		return static_cast<T*>(_components[typeId]);
	}

	template<typename T>
	inline T* GetComponent()
	{
		return static_cast<T*>(_components[T::ctype]);
	}

	GameObject();
	~GameObject();
	GameObjectPtr Clone() const;

	void AddComponent(Component* component);
	inline void SetName(std::string name) { _name = name; }
	inline void SetEnabled(bool b) { _enabled = b; }
	void SetChildren(); // sets all components' parent to this object

	inline bool GetEnabled() const { return _enabled; }
	inline Component* GetComponent(unsigned t) { return _components[t]; }
	inline Component* const * GetComponentList() { return _components; }
	inline const std::string& GetName() { return _name; }

private:
	float _life; // time (in ms) before setting this object for deletion
	bool _temporary = false;
	Component* _components[cCount];
	bool _toDestroy = false; // destroy this object at the end of the game loop
	bool _enabled; // user var
	std::string _name;

}GameObject, *GameObjectPtr;