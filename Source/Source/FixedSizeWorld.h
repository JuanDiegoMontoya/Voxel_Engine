#pragma once

struct Chunk;

class FixedSizeWorld
{
public:

	void GenWorld(glm::uvec3 chunkDim);
private:

	std::vector<Chunk*> shit;
};