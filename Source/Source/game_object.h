#pragma once
#include "component.h"

typedef class GameObject
{
public:
	template<typename T>
	inline T* GetComponent(ComponentType typeId)
	{
		return static_cast<T*>(components_[typeId]);
	}

	template<typename T>
	inline T* GetComponent()
	{
		return static_cast<T*>(components_[T::ctype]);
	}

	GameObject();
	~GameObject();
	GameObjectPtr Clone() const;

	void AddComponent(Component* component);
	inline void SetName(std::string name) { name_ = name; }
	inline void SetEnabled(bool b) { enabled_ = b; }
	void SetChildren(); // sets all components' parent to this object

	inline bool GetEnabled() const { return enabled_; }
	inline Component* GetComponent(unsigned t) { return components_[t]; }
	inline Component* const * GetComponentList() { return components_; }
	inline const std::string& GetName() { return name_; }

private:
	float life_; // time (in ms) before setting this object for deletion
	bool temporary_ = false;
	Component* components_[cCount];
	bool toDestroy_ = false; // destroy this object at the end of the game loop
	bool enabled_; // user var
	std::string name_;

}GameObject, *GameObjectPtr;