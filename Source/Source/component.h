#pragma once
//#include "game_object.h"

typedef class GameObject* GameObjectPtr;

// defines all of the component types, and in which order they are updated
enum ComponentType : unsigned
{
	cTransform,
	cCollider,
	cLight,
	cButton,
	cMesh, // encapsulates vertices and texture information theoretically
	cDynamicPhysics,
	cKinematicPhysics,
	cText,
	cScripts,
	cRenderData,

	cCount
};

// augments game objects
typedef class Component
{
public:
	inline void SetType(ComponentType t) { type_ = t; }
	inline void SetParent(GameObjectPtr p) { parent_ = p; }
	inline void SetEnabled(bool e) { enabled_ = e; }

	inline ComponentType GetType() const { return type_; }
	inline GameObjectPtr GetParent() const { return parent_; }
	inline bool GetEnabled() const { return enabled_; }

	virtual void Update(float dt) {}
	virtual ~Component() {}
	virtual Component* Clone() const { return nullptr; };

private:
	GameObjectPtr parent_;	// what object it's attached to
	ComponentType type_;		// what type of component
	bool enabled_ = true;		// user var

}Component;