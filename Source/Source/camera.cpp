#include "stdafx.h"
#include "camera.h"
#include "input.h"
#include "game_object.h"

// might add more to this constructor later
Camera::Camera(CameraType type) : type_(type) 
{
	proj_ = glm::perspective(glm::radians(fov_), 1920.f / 1080.f, 0.1f, 800.f);
}

// update movement and generate view matrix
void Camera::Update(float dt)
{
	//view_ = glm::translate(glm::mat4(1.0f), worldpos_);
	float currSpeed = speed_ * dt;
	switch (type_)
	{
	case kControlCam:
		if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
			currSpeed *= 10;
		if (Input::Keyboard().down[GLFW_KEY_W])
			worldpos_ += currSpeed * front;
		if (Input::Keyboard().down[GLFW_KEY_S])
			worldpos_ -= currSpeed * front;
		if (Input::Keyboard().down[GLFW_KEY_A])
			worldpos_ -= glm::normalize(glm::cross(front, up)) * currSpeed;
		if (Input::Keyboard().down[GLFW_KEY_D])
			worldpos_ += glm::normalize(glm::cross(front, up)) * currSpeed;

		yaw_ += Input::Mouse().screenOffset.x;
		pitch_ += Input::Mouse().screenOffset.y;

		if (pitch_ > 89.f) pitch_ = 89.f;
		if (pitch_ < -89.f) pitch_ = -89.f;

		glm::vec3 temp;
		temp.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
		temp.y = sin(glm::radians(pitch_));
		temp.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
		front = glm::normalize(temp);
		break;
	case kAffixedCam: // attached to an object, possibly follows directionality
		// TODO: add behavior to follow object
		break;
	case kImmobileCam: // do nothing
	default:
		break;
	}

	view_ = glm::lookAt(worldpos_, worldpos_ + front, up);
}