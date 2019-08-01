#pragma once
#include <functional>

class VAO;
class IBO;
class Shader;
typedef struct Chunk* ChunkPtr;
typedef std::function<void()> shaderCB;

class Renderer
{
public:
	// interation
	void DrawAll();
	void Clear();

private:
	// broad-phase rendering
	void drawShadows(); // construct shadow map(s)
	void drawNormal();	// draw what we see
	void drawPostProcessing(); // apply post processing effects

	// narrow-phase rendering
	void drawChunk(ChunkPtr chunk, shaderCB uniform_cb);
	void drawBillboard(VAO* vao, size_t count, shaderCB uniform_cb);
};