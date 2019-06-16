#include "stdafx.h"
#include "pick.h"
#include "block.h"
#include <functional>

static int wx = 1000;
static int wy = 1000;
static int wz = 1000;

BlockPtr* blocks = nullptr;

int mod(int value, int modulus)
{
	return (value % modulus + modulus) % modulus;
}

int intbound(int s, int ds)
{
	// Find the smallest positive t such that s+t*ds is an integer.
	if (ds < 0) {
		return intbound(-s, -ds);
	}
	else {
		s = mod(s, 1);
		// problem is now s+t*ds = 1
		return (1 - s) / ds;
	}
}

int signum(int x)
{
	return x > 0 ? 1 : x < 0 ? -1 : 0;
}

/**
 * Call the callback with (x,y,z,value,face) of all blocks along the line
 * segment from point 'origin' in vector direction 'direction' of length
 * 'radius'. 'radius' may be infinite.
 *
 * 'face' is the normal vector of the face of that block that was entered.
 * It should not be used after the callback returns.
 *
 * If the callback returns a true value, the traversal will be stopped.
 */
void raycast(glm::vec3 origin, glm::vec3 direction, float radius, std::function<bool(float, float, float, BlockPtr, glm::vec3)> callback)
{
	// From "A Fast Voxel Traversal Algorithm for Ray Tracing"
	// by John Amanatides and Andrew Woo, 1987
	// <http://www.cse.yorku.ca/~amana/research/grid.pdf>
	// <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
	// Extensions to the described algorithm:
	//   • Imposed a distance limit.
	//   • The face passed through to reach the current cube is provided to
	//     the callback.

	// The foundation of this algorithm is a parameterized representation of
	// the provided ray,
	//                    origin + t * direction,
	// except that t is not actually stored; rather, at any given point in the
	// traversal, we keep track of the *greater* t values which we would have
	// if we took a step sufficient to cross a cube boundary along that axis
	// (i.e. change the integer part of the coordinate) in the variables
	// tMaxX, tMaxY, and tMaxZ.

	// Cube containing origin point.
	int x = glm::floor(origin[0]);
	int y = glm::floor(origin[1]);
	int z = glm::floor(origin[2]);
	// Break out direction vector.
	int dx = direction[0];
	int dy = direction[1];
	int dz = direction[2];
	// Direction to increment x,y,z when stepping.
	int stepX = signum(dx);
	int stepY = signum(dy);
	int stepZ = signum(dz);
	// See description above. The initial values depend on the fractional
	// part of the origin.
	int tMaxX = intbound(origin[0], dx);
	int tMaxY = intbound(origin[1], dy);
	int tMaxZ = intbound(origin[2], dz);
	// The change in t when taking a step (always positive).
	int tDeltaX = stepX / dx;
	int tDeltaY = stepY / dy;
	int tDeltaZ = stepZ / dz;
	// Buffer for reporting faces to the callback.
	glm::vec3 face(0); // probably needs to point in the direction it faces

	// Avoids an infinite loop.
	if (dx == 0 && dy == 0 && dz == 0)
	{
		ASSERT_MSG(0, "Raycast in zero direction!");
	}

	// Rescale from units of 1 cube-edge to units of 'direction' so we can
	// compare with 't'.
	radius /= glm::sqrt(dx * dx + dy * dy + dz * dz);

	while (/* ray has not gone past bounds of world */
		(stepX > 0 ? x < wx : x >= 0) &&
		(stepY > 0 ? y < wy : y >= 0) &&
		(stepZ > 0 ? z < wz : z >= 0))
	{

		// Invoke the callback, unless we are not *yet* within the bounds of the
		// world.
		if (!(x < 0 || y < 0 || z < 0 || x >= wx || y >= wy || z >= wz))
			if (callback(x, y, z, blocks[x * wy * wz + y * wz + z], face))
				break;

		// tMaxX stores the t-value at which we cross a cube boundary along the
		// X axis, and similarly for Y and Z. Therefore, choosing the least tMax
		// chooses the closest cube boundary. Only the first case of the four
		// has been commented in detail.
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				if (tMaxX > radius) break;
				// Update which cube we are now in.
				x += stepX;
				// Adjust tMaxX to the next X-oriented boundary crossing.
				tMaxX += tDeltaX;
				// Record the normal vector of the cube face we entered.
				face[0] = -stepX;
				face[1] = 0;
				face[2] = 0;
			}
			else
			{
				if (tMaxZ > radius) break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		}
		else
		{
			if (tMaxY < tMaxZ) {
				if (tMaxY > radius) break;
				y += stepY;
				tMaxY += tDeltaY;
				face[0] = 0;
				face[1] = -stepY;
				face[2] = 0;
			}
			else
			{
				// Identical to the second case, repeated for simplicity in
				// the conditionals.
				if (tMaxZ > radius) break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		}
	}
}