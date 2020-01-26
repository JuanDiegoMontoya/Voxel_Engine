#include "stdafx.h"
#include "shader.h"
#include "vbo.h"
#include "ibo.h"
#include "vao.h"
#include "render.h"
#include "texture.h"
#include "lit_mesh.h"
#include "camera.h"
#include "utilities.h"
#include "level.h"
#include "render_data.h"
#include "pipeline.h"
#include "transform.h"
#include "block.h"
#include <thread>
#include "sun.h"

static void GLAPIENTRY 
GLerrorCB(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
	//return; // UNCOMMENT WHEN DEBUGGING GRAPHICS

	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

namespace Render
{
	std::unordered_map<const char*, Texture*, Utils::djb2hash, Utils::charPtrKeyEq> textures;
	Camera* currCamera;
	ShaderPtr currShader;
	Renderer renderer;
	LevelPtr currLevel = nullptr;
	
	void CalcModels(int low, int high)
	{
		for (int i = low; i < high; i++)
		{
			//modelsMAT[updatedBlocks[i]] = Block::blocksarr_[updatedBlocks[i]].GetModel();
			//colorsVEC[updatedBlocks[i]] = Block::blocksarr_[updatedBlocks[i]].clr;
		}
	}

	void Init()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEPTH_TEST);

		// enable debugging stuff
		glDebugMessageCallback(GLerrorCB, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// initialize all of the shaders that will be used
		Shader::shaders["postprocess"] = new Shader("postprocess.vs", "postprocess.fs");
		Shader::shaders["flat"] = new Shader("flat_color_instanced.vs", "flat_color_instanced.fs");
		Shader::shaders["chunk"] = new Shader("chunk_flat.vs", "chunk_flat.fs");
		Shader::shaders["sun"] = new Shader("flat_sun.vs", "flat_sun.fs");
		Shader::shaders["shadow"] = new Shader("shadow.vs", "shadow.fs");
		Shader::shaders["axis"] = new Shader("axis.vs", "axis.fs");
		Shader::shaders["flat_color"] = new Shader("flat_color.vs", "flat_color.fs");

		Shader::shaders["chunk_water"] = new Shader("chunk_water.vs", "chunk_water.fs");
		Shader::shaders["chunk_shaded"] = new Shader("chunk_smooth_light.vs", "chunk_smooth_light.fs");
		Shader::shaders["chunk_geometry"] = new Shader("chunk_gBuffer.vs", "chunk_gBuffer.fs");
		std::vector<int> values = { 0, 1, 2 };
		//std::vector<int> values = { 1, 2, 3 };
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[0]", 0);
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[1]", 1);
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[2]", 2);
		Shader::shaders["chunk_shaded"]->Use();
		Shader::shaders["chunk_shaded"]->setIntArray("shadowMap", values, values.size());
		Shader::shaders["chunk_water"]->Use();
		Shader::shaders["chunk_water"]->setIntArray("shadowMap", values, values.size());
		Shader::shaders["chunk_water"]->setInt("ssr_positions", 3);
		//Shader::shaders["chunk_water"]->setInt("ssr_normals", 4);
		Shader::shaders["chunk_water"]->setInt("ssr_albedoSpec", 5);
		//Shader::shaders["chunk_water"]->setInt("ssr_depth", 6);


		Shader::shaders["debug_map3"] = new Shader("debug_map.vs", "debug_map.fs");
		Shader::shaders["debug_shadow"] = new Shader("debug_shadow.vs", "debug_shadow.fs");
		Shader::shaders["debug_shadow"]->Use();
		//Shader::shaders["debug_shadow"]->setInt("depthMap", 1);
	}

	void SetCamera(Camera* cam)
	{
		currCamera = cam;
	}

	Camera* GetCamera()
	{
		return currCamera;
	}

	void drawImGui()
	{
		ImGui::Begin("Giraffix");

		ImGui::End();
	}

	void Terminate()
	{

	}
}