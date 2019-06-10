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

typedef class Component
{
public:

	inline ComponentType GetType() const { return _type; };
	inline void SetType(ComponentType t) { _type = t; };
	virtual void Update(float dt) {};
	virtual ~Component() {}
	virtual Component* Clone() = delete;

private:
	ComponentType _type;
	bool _enabled = true;
};