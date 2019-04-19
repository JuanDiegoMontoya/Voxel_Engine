#include "stdafx.h"
#include <stb_image.h>

unsigned char* loadImage(const char* name)
{
	int width, height, n;
	return (unsigned char*)stbi_load(name, &width, &height, &n, 4);
}

void deleteImage(unsigned char* image)
{
	stbi_image_free(image);
}