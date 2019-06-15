#pragma once

typedef class RenderData : public Component
{
public:
	RenderData() { SetType(cRenderData); }
	~RenderData() {}
	Component* Clone() const override;

	void UseUntexturedBlockData(); // draw as unreflective, colored block

	inline void SetIsTextured(bool b) { _isTextured = b; }
	inline void SetTexture(Texture* t) { _texture = t; }
	inline void SetColor(glm::vec4 c) { _color = c; }

	inline bool GetIsTextured() const { return _isTextured; }
	inline glm::vec4 GetColor() const { return _color; }
	inline const VAO& GetVao() { return *_vao; }
	inline const VBO& GetVbo() { return *_vbo; }
	inline const IBO& GetIbo() { return *_ibo; }
	inline ShaderPtr GetShader() { return _shader; } // not const so we can modify it

	static const ComponentType ctype = cRenderData;

private:
	// do NOT delete these upon destruction of this object
	VAO* _vao = nullptr;
	VBO* _vbo = nullptr;
	IBO* _ibo = nullptr;

	Texture* _texture = nullptr;
	ShaderPtr _shader = nullptr;
	bool _isTextured = false;
	glm::vec4 _color = glm::vec4(1.f); // if not textured

}RenderData, *RenderDataPtr;