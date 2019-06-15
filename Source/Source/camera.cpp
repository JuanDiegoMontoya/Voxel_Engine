#include "stdafx.h"
#include "camera.h"
#include "input.h"
#include "game_object.h"

// might add more to this constructor later
Camera::Camera(CameraType type) : _type(type) 
{
	_proj = glm::perspective(glm::radians(_fov), 1920.f / 1080.f, 0.1f, 800.f);
}

// update movement and generate view matrix
void Camera::Update(float dt)
{
	//_view = glm::translate(glm::mat4(1.0f), _worldpos);
	_view = glm::lookAt(_worldpos, _worldpos + front, up);
	float currSpeed = _speed * dt;
	switch (_type)
	{
	case kControlCam:
		if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
			currSpeed *= 5;
		if (Input::Keyboard().down[GLFW_KEY_W])
			_worldpos += currSpeed * front;
		if (Input::Keyboard().down[GLFW_KEY_S])
			_worldpos -= currSpeed * front;
		if (Input::Keyboard().down[GLFW_KEY_A])
			_worldpos -= glm::normalize(glm::cross(front, up)) * currSpeed;
		if (Input::Keyboard().down[GLFW_KEY_D])
			_worldpos += glm::normalize(glm::cross(front, up)) * currSpeed;

		_yaw += Input::Mouse().screenOffset.x;
		_pitch += Input::Mouse().screenOffset.y;

		if (_pitch > 89.f) _pitch = 89.f;
		if (_pitch < -89.f) _pitch = -89.f;

		glm::vec3 temp;
		temp.x = cos(glm::radians(_pitch)) * cos(glm::radians(_yaw));
		temp.y = sin(glm::radians(_pitch));
		temp.z = cos(glm::radians(_pitch)) * sin(glm::radians(_yaw));
		front = glm::normalize(temp);
		break;
	case kAffixedCam: // attached to an object, possibly follows directionality
		// TODO: add behavior to follow object
		break;
	case kImmobileCam: // do nothing
	default:
		break;
	}
}