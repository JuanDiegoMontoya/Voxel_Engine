#pragma once

class TextureArray
{
public:
	TextureArray(const std::vector<const char*> textures);
	~TextureArray();

	void Bind(GLuint slot) const;
	void Unbind() const;

private:

	GLuint id_ = 0;
	const GLsizei mipCount_ = 1;
	const int dim = 32;
	const std::string texPath = "./resources/Textures/";
};