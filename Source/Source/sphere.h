#pragma once
#include "stdafx.h"

class SolidSphere
{
public:
	std::vector<GLfloat> vertex;

//private:
	std::vector<GLfloat> positions;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> texCoords;
	std::vector<GLuint> indices;

public:
	SolidSphere(float radius, unsigned int stackCount, unsigned int sectorCount)
	{
		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = 2 * glm::pi<float>() / sectorCount;
		float stackStep = glm::pi<float>() / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = glm::pi<float>() / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);             // r * cos(u)
			z = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
				positions.push_back(x);
				positions.push_back(y);
				positions.push_back(z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				normals.push_back(nx);
				normals.push_back(ny);
				normals.push_back(nz);

				// vertex tex coord (s, t) range between [0, 1]
				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				texCoords.push_back(s);
				texCoords.push_back(t);
			}
		}

		// generate CCW index list of sphere triangles
		int k1, k2;
		for (int i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if (i != (stackCount - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		vertex.reserve(positions.size() + normals.size() + texCoords.size());
		for (size_t i = 0; i < positions.size() + normals.size() + texCoords.size(); i++)
		{
			vertex.push_back(0);
		}

		// manually construct a list of vertices. Efficient? Probably not.
		for (size_t i = 0; i < vertex.size(); i += 8)
		{
			vertex[i + 0] = positions[i / 8 * 3 + 0];
			vertex[i + 1] = positions[i / 8 * 3 + 1];
			vertex[i + 2] = positions[i / 8 * 3 + 2];
			vertex[i + 3] = normals  [i / 8 * 3 + 0];
			vertex[i + 4] = normals  [i / 8 * 3 + 1];
			vertex[i + 5] = normals  [i / 8 * 3 + 2];
			vertex[i + 6] = texCoords[i / 8 * 2 + 0];
			vertex[i + 7] = texCoords[i / 8 * 2 + 1];
		}
	}
};