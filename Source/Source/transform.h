#pragma once
#include "component.h"

typedef class Transform : public Component
{
public:
	Transform(const glm::vec3& pos = glm::vec3(0.0f), 
		const glm::vec3& scale = glm::vec3(1.0f));
	Transform(const Transform & other);
	~Transform() {};

	void SetTranslation(const glm::vec3& translation);
	void SetRotation(const glm::mat4& rotation);
	void SetScale(const glm::vec3& scale);

	inline const glm::vec3& GetTranslation() const { return _translation; };
	inline const glm::mat4& GetRotation() const { return _rotation; };
	inline const glm::vec3& GetScale() const { return _scale; };
	const glm::mat4& GetModel() const;
	Component* Clone() const override;

	static const ComponentType ctype = cTransform;

private:
	glm::vec3	_translation;
	glm::mat4 _rotation;
	glm::vec3	_scale;

	mutable glm::mat4	_model;
	mutable bool			_dirty;

}Transform, TransformPtr;