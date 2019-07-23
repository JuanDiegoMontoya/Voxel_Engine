#pragma once

class VAO;
class VBO;
class Frustum;

// right, left, bottom, top, far, near
struct BasicFrustum
{
	float r, l, b, t, f, n;
};

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
	inline const glm::mat4& GetView() { return view_; }
	inline const GLuint GetDepthFBO() const { return depthMapFBO_; }
	inline const glm::uvec3& GetDepthTex() const { return depthMapTexes_; }
	inline const float GetNearPlane() const { return near_; }
	inline const float GetFarPlane() const { return far_; }
	inline const Frustum* GetFrustum() const { return frustum_; }
	inline const unsigned GetNumCascades() const { return shadowCascades_; }
	inline const glm::vec4& GetCascadeEnds() const { return cascadeEnds_; }
	inline const glm::mat4* GetShadowOrthoProjMtxs() const { return shadowOrthoProjMtxs_; }

	inline void SetPos(const glm::vec3& pos) { pos_ = pos; }
	inline void SetDir(const glm::vec3& dir) { dir_ = dir; }
	inline void SetNearPlane(float f) { near_ = f; }
	inline void SetFarPlane(float f) { far_ = f; }
	inline void SetShadowSize(const glm::ivec2& s) { shadowSize_ = s; }
	inline void SetNumCascades(unsigned num) { shadowCascades_ = num; }

	bool orbits = false;
	glm::vec3 orbitPos = glm::vec3(0);

	bool followCam = true;
	float followDist = 75.f;
	
	float projSize = 100.f;

	void bindForWriting(unsigned index);
	void bindForReading();
private:
	void initCascadedShadowMapFBO();
	void calcOrthoProjs();

	glm::vec3 dir_;
	glm::vec3 pos_; // position relative to camera

	// shadow related
	glm::mat4 sunViewProj_;
	glm::mat4 view_;
	glm::ivec2 shadowSize_ = glm::ivec2(2048, 2048);
	GLuint depthMapFBO_;
	unsigned shadowCascades_ = 3; // 3 = max (uvec3)
	glm::uvec3 depthMapTexes_;
	glm::vec4 cascadeEnds_;
	float near_;
	float far_;

	Frustum* frustum_;
	//BasicFrustum shadowOrthoProjInfo_[3];
	glm::mat4 shadowOrthoProjMtxs_[3];

	VAO* vao_;
	VBO* vbo_;
};