#include "stdafx.h"
#include "TextureArray.h"
#include <stb_image.h>
#include <filesystem>

// maybe get rid of this include
//#include "block.h"

//#pragma optimize("", off)
TextureArray::TextureArray(const std::vector<std::string_view>& textures)
{
	const GLsizei layerCount = textures.size();
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipCount_, GL_RGB8, dim, dim, layerCount);

	stbi_set_flip_vertically_on_load(true);

	int i = 0;
	for (auto texture : textures)
	{
		std::string tex = texPath + texture.data();
		bool hasTex = std::filesystem::exists(texPath + texture.data());
		hasTex = false;

		// TODO: gen mipmaps (increment mip level each mip iteration)
		if (hasTex == false)
		{
			std::cout << "Failed to load texture " << texture << ", using fallback.\n";
			tex = texPath + "error.png";
		}

		int width, height, n;
		auto pixels = (unsigned char*)stbi_load(tex.c_str(), &width, &height, &n, 4);
		ASSERT(pixels != nullptr);
		ASSERT(width == dim && height == dim);

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,           // mip level 0
			0, 0, i,     // image start layer
			dim, dim, 1, // x, y, z size (z = 1 b/c it's just a single layer)
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			pixels);

		//for (int i = 0; i < mipCount_; i++)
		{
			//glTexImage3D(GL_TEXTURE_2D_ARRAY, 
			//	i, 
			//	GL_RGB8, 
			//	dim / (i * 2), dim / (i * 2), layerCount, 
			//	0, 
			//	GL_RGBA,
			//	GL_UNSIGNED_BYTE,)
		}

		stbi_image_free(pixels);

		//else // the given texture wasn't found- use a replacement
		//{
		//	// checkered 2x2 purple texture
		//	GLubyte texels[] =
		//	{
		//		255, 0, 255,
		//		0, 0, 0,
		//		255, 255, // these do nothing?
		//		0, 0, 0,
		//		255, 0, 255,
		//	};

		//	GLubyte texels2[4 * 4];
		//	for (int i = 0; i < 4 * 4; i++)
		//	{

		//	}

		//	glTexSubImage3D(
		//		GL_TEXTURE_2D_ARRAY, 
		//		0, 
		//		0, 0, i, 
		//		dim, dim, 1, 
		//		GL_RGB, 
		//		GL_UNSIGNED_BYTE, 
		//		texels);
		//}

		i++;
	}

	// sets the anisotropic filtering texture paramter to the highest supported by the system
	// TODO: make this parameter user-selectable
	GLint a;
	glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &a);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY, a);

	// TODO: play with this parameter for optimal looks, maybe make it user-selectable
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// use OpenGL to generate mipmaps for us
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
//#pragma optimize("", on)


TextureArray::~TextureArray()
{
	glDeleteTextures(1, &id_);
}


void TextureArray::Bind(GLuint slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
}


void TextureArray::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}
