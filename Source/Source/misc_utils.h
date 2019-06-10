#pragma once
#include "utilities.h"

namespace Utils
{
	struct djb2hash
	{
		size_t operator()(const char* cp) const
		{
			size_t hash = 5381;
			while (*cp)
				hash = 33 * hash ^ (unsigned char)*cp++;
			return hash;
		}
	};

	struct charPtrKeyEq
	{
		bool operator()(const char* first, const char* second) const
		{
			return !strcmp(first, second);
		}
	};

	/**
	 * Converts an RGB color value to HSL. Conversion formula
	 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
	 * Assumes r, g, and b are contained in the set [0, 255] and
	 * returns h, s, and l in the set [0, 1].
	 *
	 * @param   {number}  r       The red color value
	 * @param   {number}  g       The green color value
	 * @param   {number}  b       The blue color value
	 * @return  {Array}           The HSL representation
	 */
	glm::vec3 RGBtoHSL(glm::vec3 rgb);

	/**
	 * Converts an HSL color value to RGB. Conversion formula
	 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
	 * Assumes h, s, and l are contained in the set [0, 1] and
	 * returns r, g, and b in the set [0, 255].
	 *
	 * @param   {number}  h       The hue
	 * @param   {number}  s       The saturation
	 * @param   {number}  l       The lightness
	 * @return  {Array}           The RGB representation
	 */
	glm::vec3 HSLtoRGB(glm::vec3 hsl);
}