#pragma once

class VAO;
class VBO;

class Sun
{
public:
	Sun();

	void Update();
	void Render();

	// getters
	inline const glm::vec3& GetDir() const { return dir_; }
	inline const glm::vec3& GetPos() const { return pos_; }
	inline const glm::ivec2& GetShadowSize() const { return shadowSize_; }
	inline const glm::mat4& GetViewProj() { return sunViewProj_; }
	inline const GLuint GetDepthFBO() const { return depthMapFBO_; }
	inline const GLuint GetDepthTex() const { return depthMapTex_; }
	inline const float GetNearPlane() const { return near_; }
	inline const float GetFarPlane() const { return far_; }

	inline void SetPos(const glm::vec3& pos) { pos_ = pos; }
	inline void SetDir(const glm::vec3& dir) { dir_ = dir; }
	inline void SetNearPlane(float f) { near_ = f; }
	inline void SetFarPlane(float f) { far_ = f; }
	inline void SetShadowSize(const glm::ivec2& s) { shadowSize_ = s; }

	bool orbits = false;
	glm::vec3 orbitPos = glm::vec3(0);

	bool followCam = false;
	float followDist = 200.f;

	float projSize = 200.f;
private:
	glm::vec3 dir_;
	glm::vec3 pos_; // position relative to camera

	// shadow related
	glm::mat4 sunViewProj_;
	glm::ivec2 shadowSize_ = glm::ivec2(8192, 8192);
	GLuint depthMapFBO_;
	GLuint depthMapTex_;
	float near_;
	float far_;

	VAO* vao_;
	VBO* vbo_;
};