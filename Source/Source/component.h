#pragma once

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
	inline void SetType(ComponentType t) { _type = t; }
	inline void SetParent(GameObjectPtr p) { _parent = p; }

	inline ComponentType GetType() const { return _type; }
	inline GameObjectPtr Parent() const { return _parent; };

	virtual void Update(float dt) {}
	virtual ~Component() {}
	virtual Component* Clone() = delete;

private:
	GameObjectPtr _parent;	// what object it's attached to
	ComponentType _type;		// what type of component
	bool _enabled = true;		// user var

}Component;