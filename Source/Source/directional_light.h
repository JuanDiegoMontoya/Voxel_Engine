#pragma once

class VAO;
class VBO;

/*
	Orthographic parallel-rays light (such as the sun)
	with cascaded shadows. 
*/
class DirLight
{
public:
	DirLight();

	void Update(const glm::vec3& pos, const glm::vec3& dir);

	// getters
	inline const glm::vec3& GetDir() const { return tDir_; }
	inline const glm::vec3& GetPos() const { return tPos_; }
	inline const glm::ivec2& GetShadowSize() const { return shadowSize_; }
	inline const glm::mat4& GetView() { return view_; }
	inline const GLuint GetDepthFBO() const { return depthMapFBO_; }
	inline const glm::uvec3& GetDepthTex() const { return depthMapTexes_; }
	inline const unsigned GetNumCascades() const { return shadowCascades_; }
	inline const glm::vec4& GetCascadeEnds() const { return cascadeEnds_; }
	inline const glm::mat4* GetShadowOrthoProjMtxs() const { return shadowOrthoProjMtxs_; }

	// setters
	inline void SetDir(const glm::vec3& dir) { tDir_ = dir; }
	inline void SetPos(const glm::vec3& pos) { tPos_ = pos; }
	inline void SetShadowSize(const glm::ivec2& s) { shadowSize_ = s; }
	inline void SetNumCascades(unsigned num) { shadowCascades_ = num; }

	friend class Renderer;
private:
	// functions
	void initCascadedShadowMapFBO();
	void bindForWriting(unsigned index);
	void bindForReading();
	void calcOrthoProjs();
	void calcPersProjs();

	// vars
	glm::vec3 tDir_;
	glm::vec3 tPos_;
	glm::mat4 view_;
	glm::ivec2 shadowSize_ = glm::ivec2(2048, 2048);
	GLuint depthMapFBO_;
	unsigned shadowCascades_ = 3; // 3 = max (uvec3)
	glm::uvec3 depthMapTexes_;
	glm::vec4 cascadeEnds_;

	glm::mat4 shadowOrthoProjMtxs_[3];
};