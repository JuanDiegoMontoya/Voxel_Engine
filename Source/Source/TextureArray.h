#pragma once

class TextureArray
{
public:
	TextureArray(const std::vector<std::string_view>& textures);
	~TextureArray();

	void Bind(GLuint slot) const;
	void Unbind() const;

private:

	GLuint id_ = 0;
	const GLsizei mipCount_ = 6; // log2(32) + 1 (base texture)
	const int dim = 32;
	const std::string texPath = "./resources/Textures/";
};