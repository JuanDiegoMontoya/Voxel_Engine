#include "stdafx.h"
#include "TextureArray.h"
#include <stb_image.h>
#include <filesystem>

// maybe get rid of this include
#include "block.h"


TextureArray::TextureArray(const std::vector<const char*> textures)
{
	const GLsizei layerCount = textures.size();
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D_ARRAY, id_);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipCount_, GL_RGB8, dim, dim, layerCount);

	int i = 0;
	for (auto tex : textures)
	{
		bool hasTex = std::filesystem::exists(texPath + tex);

		// TODO: gen mipmaps (increment mip level each iteration)
		if (hasTex)
		{
			int width, height, n;
			auto pixels = (unsigned char*)stbi_load(tex, &width, &height, &n, 3);
			ASSERT(width == dim && height == dim);


			glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY, 
				0,           // mip level 0
				0, 0, i,     // image start layer
				dim, dim, 1, // x, y, z size (z = 1 b/c it's just a single layer)
				GL_RGB, 
				GL_UNSIGNED_BYTE, 
				pixels);

			stbi_image_free(pixels);
		}
		else // the given texture wasn't found- use a replacement
		{
			// checkered 2x2 purple texture
			const GLubyte texels[] =
			{
				255, 0, 255,
				0, 0, 0,
				0, 0, 0,
				255, 0, 255,
			};

			glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY, 
				0, 
				0, 0, i, 
				2, 2, 1, 
				GL_RGB, 
				GL_UNSIGNED_BYTE, 
				texels);
		}
		i++;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


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
