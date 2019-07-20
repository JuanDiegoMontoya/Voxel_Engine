#pragma once
#include "vendor/noiseutils.h"

// TODO: ngl this is probably a useless class
class Superchunk
{
public:
	static void GenerateSuperchunkImage(glm::ivec3 regionPos);

	static std::unordered_map<glm::ivec3, utils::Image, Utils::ivec3Hash> superchunks;
	static constexpr unsigned SUPERCHUNK_SIZE = 512;
private:
};