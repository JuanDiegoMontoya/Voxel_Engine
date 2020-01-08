#pragma once
#include <functional>

class VAO;
class IBO;
class Shader;
class DirLight;
class Sun;
class ChunkManager;
typedef struct Chunk* ChunkPtr;
typedef std::function<void()> DrawCB;
typedef std::function<void(const glm::mat4&)> ModelCB;

// responsible for making stuff appear on the screen
class Renderer
{
public:
	Renderer();
	void Init();

	// interation
	void DrawAll();
	void Clear();
	void ClearCSM();
	void ClearGGuffer();

	void SetDirLight(DirLight* d) { activeDirLight_ = d; }
	void SetSun(Sun* s) { activeSun_ = s; }

	static void DrawCube();

	ChunkManager* chunkManager_;

	bool renderShadows = true;
	bool doGeometryPass = true; // for ssr currently
	//bool renderSSR = true;

	// pp effects
	bool ppSharpenFilter = false;
	bool ppBlurFilter = false;
	bool ppEdgeDetection = false;
	bool ppChromaticAberration = false;
private:
	// broad-phase rendering
	void drawShadows(); // construct shadow map(s)
	void drawSky();
	void drawNormal();	// draw what we see
	void drawWater();
	void drawPostProcessing(); // apply post processing effects
	void drawDebug();

	// narrow-phase rendering
	void drawChunks(bool cullFrustum, 
		DrawCB predraw_cb, 
		ModelCB draw_cb, 
		DrawCB postdraw_cb);
	void drawBillboard(VAO* vao, size_t count, DrawCB uniform_cb);
	void drawQuad();

	// debug
	void drawDepthMapsDebug();
	void drawAxisIndicators();

	// deferred rendering
	void initDeferredBuffers();
	void geometryPass();
	void lightingPass();

	// post processing
	void initPPBuffers();
	void postProcess();

	DirLight* activeDirLight_;
	Sun* activeSun_;

	// ssr & deferred rendering
	unsigned gBuffer; // framebuffer
	unsigned gPosition, gNormal, gAlbedoSpec, gDepth;
	unsigned rboDepth; // depth renderbuffer

	// pp
	unsigned pBuffer;
	unsigned pColor;
	unsigned pDepth;


	// CSM (temp?)
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 vView[3];							// 
	glm::vec3 LitDir;								// direction of light
	glm::vec3 right;								// light view right
	glm::vec3 up;										// light view up
	glm::mat4 LitViewFam;						// light space view matrix
	glm::vec3 ratios;								// unused (exponential shadows)
	glm::vec4 cascadEnds;						// light space or something
	glm::vec3 cascadeEndsClipSpace; // clip space
};