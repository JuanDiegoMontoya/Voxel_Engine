#pragma once

enum CameraType : int
{
	kControlCam, // use movement keys to fly around
	kAffixedCam, // parented to some object
	kImmobileCam // cannot move
};

class Camera
{
public:
	Camera(CameraType type);
	void Update(float dt);
	const glm::mat4& GetView() { return _view; }
	const glm::mat4& GetProj() { return _proj; }

	glm::vec3 up = glm::vec3(0, 1.f, 0);
	glm::vec3 front = glm::vec3(0, 0, -1.f);
private:
	CameraType _type;
	glm::vec3 _worldpos = glm::vec3(0, 0, 3.f);
	glm::mat4 _view = glm::mat4(1);
	glm::mat4 _proj;

	float _speed = 3.5f;

	float _pitch = 0;
	float _yaw = -90.0f;
	float _roll = 0;
	float _fov = 70.f;
};