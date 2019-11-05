#pragma once
#include "component.h"

class Frustum;

enum CameraType : int
{
	kPhysicsCam, // uses movement keys to affect velocity and/or acceleration
	kControlCam, // use movement keys to fly around
	kAffixedCam, // parented to some object
	kImmobileCam // cannot move
};

class Camera : public Component
{
public:
	Camera(CameraType type);
	void Update(float dt);
	const glm::mat4& GetView() const { return view_; }
	const glm::mat4& GetProj() const { return proj_; }
	const glm::vec3& GetPos() const { return worldpos_; }
	const glm::vec3& GetDir() const { return dir_; }
	const Frustum* GetFrustum() const { return frustum_; }
	const float GetFov() const { return fov_; }
	const float GetNear() const { return near_; }
	const float GetFar() const { return far_; }
	const CameraType GetType() const { return type_; }

	void SetPos(const glm::vec3& v) { worldpos_ = v; }
	void SetType(CameraType t) { type_ = t; }

	glm::vec3 up = glm::vec3(0, 1.f, 0);
	glm::vec3 front = glm::vec3(0, 0, -1.f);

	// physics (temp)
	glm::vec3 velocity_;
	glm::vec3 acceleration_;
	float maxspeed_ = 5.0f;
	glm::vec3 oldPos;
private:
	CameraType type_;
	glm::vec3 worldpos_ = glm::vec3(0, 10, 0);
	glm::vec3 dir_ = glm::vec3(0, 0, 0);
	glm::mat4 view_ = glm::mat4(1);
	glm::mat4 proj_;
	Frustum* frustum_;

	float speed_ = 3.5f;

	float pitch_ = 0;
	float yaw_ = -90.0f;
	float roll_ = 0;
	float fov_ = 80.f;

	float near_ = .1f;
	float far_ = 500.f;
};