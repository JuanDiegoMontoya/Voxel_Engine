#pragma once

namespace NuRenderer
{
	void Init();
	void CompileShaders();
	void Clear();
	void DrawAll();

	void drawChunks();
	void splatChunks();
	void drawChunksWater();
}