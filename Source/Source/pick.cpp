#include "stdafx.h"
#include <functional>
#include "block.h"
#include "chunk.h"
#include "pick.h"

static int ww = 10000;

BlockPtr* blocks = nullptr;

float mod(float value, float modulus)
{
	return fmod((fmod(value, modulus)) + modulus, modulus);
	//return (value % modulus + modulus) % modulus;
}

float intbound(float s, float ds)
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

int signum(float x)
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
	int x = int(glm::floor(origin[0]));
	int y = int(glm::floor(origin[1]));
	int z = int(glm::floor(origin[2]));
	// Break out direction vector.
	float dx = direction[0];
	float dy = direction[1];
	float dz = direction[2];
	// Direction to increment x,y,z when stepping.
	int stepX = signum(dx);
	int stepY = signum(dy);
	int stepZ = signum(dz);
	// See description above. The initial values depend on the fractional
	// part of the origin.
	float tMaxX = intbound(origin[0], dx);
	float tMaxY = intbound(origin[1], dy);
	float tMaxZ = intbound(origin[2], dz);
	// The change in t when taking a step (always positive).
	float tDeltaX = stepX / dx;
	float tDeltaY = stepY / dy;
	float tDeltaZ = stepZ / dz;
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

	while (1)
	{

		// Invoke the callback, unless we are not *yet* within the bounds of the
		// world.
		if (callback(float(x), float(y), float(z), Chunk::AtWorld(glm::ivec3(x, y, z)), face))
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
				face[0] = float(-stepX);
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
				face[2] = float(-stepZ);
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				if (tMaxY > radius) break;
				y += stepY;
				tMaxY += tDeltaY;
				face[0] = 0;
				face[1] = float(-stepY);
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
				face[2] = float(-stepZ);
			}
		}
	}
}

// Bresenham3D
//
// A slightly modified version of the source found at
// http://www.ict.griffith.edu.au/anthony/info/graphics/bresenham.procs
// Provided by Anthony Thyssen, though he does not take credit for the original implementation
//
// It is highly likely that the original Author was Bob Pendelton, as referenced here
//
// ftp://ftp.isc.org/pub/usenet/comp.sources.unix/volume26/line3d
//
// line3d was dervied from DigitalLine.c published as "Digital Line Drawing"
// by Paul Heckbert from "Graphics Gems", Academic Press, 1990
//
// 3D modifications by Bob Pendleton. The original source code was in the public
// domain, the author of the 3D version places his modifications in the
// public domain as well.
//
// line3d uses Bresenham's algorithm to generate the 3 dimensional points on a
// line from (x1, y1, z1) to (x2, y2, z2)


//void raycast(glm::vec3 origin, glm::vec3 direction, float radius, 
//	std::function<bool(float, float, float, BlockPtr, glm::vec3)> callback)
//{
//	int i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc, err_1, err_2, dx2, dy2, dz2;
//	//int point[3];
//	int x1, y1, z1;
//	int x2, y2, z2;
//	glm::ivec3 face(0);
//	glm::ivec3 point;
//	x1 = origin.x;
//	y1 = origin.y;
//	z1 = origin.z;
//	x2 = x1 + direction.x * radius;
//	y2 = y1 + direction.y * radius;
//	z2 = z1 + direction.z * radius;
//
//	point[0] = x1;
//	point[1] = y1;
//	point[2] = z1;
//	dx = x2 - x1;
//	dy = y2 - y1;
//	dz = z2 - z1;
//	x_inc = (dx < 0) ? -1 : 1;
//	l = abs(dx);
//	y_inc = (dy < 0) ? -1 : 1;
//	m = abs(dy);
//	z_inc = (dz < 0) ? -1 : 1;
//	n = abs(dz);
//	dx2 = l << 1;
//	dy2 = m << 1;
//	dz2 = n << 1;
//
//	if ((l >= m) && (l >= n))
//	{
//		err_1 = dy2 - l;
//		err_2 = dz2 - l;
//		for (i = 0; i < l; i++)
//		{
//			//output->getTileAt(point[0], point[1], point[2])->setSymbol(symbol);
//			if (callback(point.x, point.y, point.z, Chunk::AtWorld(point), face))
//				break;
//			if (err_1 > 0)
//			{
//				point[1] += y_inc;
//				err_1 -= dx2;
//			}
//			if (err_2 > 0)
//			{
//				point[2] += z_inc;
//				err_2 -= dx2;
//			}
//			err_1 += dy2;
//			err_2 += dz2;
//			point[0] += x_inc;
//		}
//	}
//	else if ((m >= l) && (m >= n))
//	{
//		err_1 = dx2 - m;
//		err_2 = dz2 - m;
//		for (i = 0; i < m; i++)
//		{
//			//output->getTileAt(point[0], point[1], point[2])->setSymbol(symbol);
//			if (callback(point.x, point.y, point.z, Chunk::AtWorld(point), face))
//				return;
//			if (err_1 > 0) {
//				point[0] += x_inc;
//				err_1 -= dy2;
//			}
//			if (err_2 > 0)
//			{
//				point[2] += z_inc;
//				err_2 -= dy2;
//			}
//			err_1 += dx2;
//			err_2 += dz2;
//			point[1] += y_inc;
//		}
//	}
//	else
//	{
//		err_1 = dy2 - n;
//		err_2 = dx2 - n;
//		for (i = 0; i < n; i++)
//		{
//			//output->getTileAt(point[0], point[1], point[2])->setSymbol(symbol);
//			if (callback(point.x, point.y, point.z, Chunk::AtWorld(point), face))
//				return;
//			if (err_1 > 0) {
//				point[1] += y_inc;
//				err_1 -= dz2;
//			}
//			if (err_2 > 0)
//			{
//				point[0] += x_inc;
//				err_2 -= dz2;
//			}
//			err_1 += dy2;
//			err_2 += dx2;
//			point[2] += z_inc;
//		}
//	}
//	//output->getTileAt(point[0], point[1], point[2])->setSymbol(symbol);
//	callback(point.x, point.y, point.z, Chunk::AtWorld(point), face);
//}

//void raycast(glm::vec3 origin, glm::vec3 direction, float radius, std::function<bool(float, float, float, BlockPtr, glm::vec3)> callback)
//{
//	// r.dir is unit direction vector of ray
//	dirfrac.x = 1.0f / direction.x;
//	dirfrac.y = 1.0f / direction.y;
//	dirfrac.z = 1.0f / direction.z;
//
//	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
//	// r.org is origin of ray
//	float t1 = (lb.x - r.org.x) * dirfrac.x;
//	float t2 = (rt.x - r.org.x) * dirfrac.x;
//	float t3 = (lb.y - r.org.y) * dirfrac.y;
//	float t4 = (rt.y - r.org.y) * dirfrac.y;
//	float t5 = (lb.z - r.org.z) * dirfrac.z;
//	float t6 = (rt.z - r.org.z) * dirfrac.z;
//
//	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
//	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));
//
//	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
//	if (tmax < 0)
//	{
//		t = tmax;
//		return false;
//	}
//
//	// if tmin > tmax, ray doesn't intersect AABB
//	if (tmin > tmax)
//	{
//		t = tmax;
//		return false;
//	}
//
//	t = tmin;
//	return true;
//}