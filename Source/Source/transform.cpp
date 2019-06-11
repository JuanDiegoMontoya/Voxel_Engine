#include "stdafx.h"
#include "transform.h"

Transform::Transform(const glm::vec3& pos, const glm::vec3& scale)
{
	_translation = pos;
	_rotation = glm::mat4(1.f);
	_scale = scale;
	_model = glm::mat4(1.0f);
	_dirty = true;
	SetType(cTransform);
}

Transform::Transform(const Transform& other)
{
	*this = other;
	SetType(cTransform);
}

Component* Transform::Clone() const
{
	return (Component*)new Transform(*this);
}

void Transform::SetTranslation(const glm::vec3& translation)
{
	_translation = translation;
	_dirty = true;
}

void Transform::SetRotation(const glm::mat4& rotation)
{
	_rotation = rotation;
	_dirty = true;
}

void Transform::SetScale(const glm::vec3& scale)
{
	_scale = scale;
	_dirty = true;
}

// computes the model matrix if necessary, then returns it
const glm::mat4& Transform::GetModel() const
{
	if (_dirty)
	{
		_model = glm::mat4(1.0f);
		_model = glm::translate(_model, _translation);
		_model *= _rotation;
		_model = glm::scale(_model, _scale);

		_dirty = false;
	}

	return _model;
}