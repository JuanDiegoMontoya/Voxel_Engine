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

#define ID2D(x, y, w) (width * row + col)

static void GLAPIENTRY 
GLerrorCB(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
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
		Shader::shaders["flat"] = new Shader("flat_color_instanced.vs", "flat_color_instanced.fs");
		Shader::shaders["chunk"] = new Shader("chunk_flat.vs", "chunk_flat.fs");
		Shader::shaders["sun"] = new Shader("flat_sun.vs", "flat_sun.fs");
		Shader::shaders["shadow"] = new Shader("shadow.vs", "shadow.fs");
		Shader::shaders["axis"] = new Shader("axis.vs", "axis.fs");
		Shader::shaders["flat_color"] = new Shader("flat_color.vs", "flat_color.fs");

		Shader::shaders["chunk_shaded"] = new Shader("chunk_smooth_light.vs", "chunk_smooth_light.fs");
		Shader::shaders["chunk_shaded"]->Use();
		Shader::shaders["chunk_shaded"]->setInt("shadowMap", 0);

		Shader::shaders["debug_shadow"] = new Shader("debug_shadow.vs", "debug_shadow.fs");
		Shader::shaders["debug_shadow"]->Use();
		Shader::shaders["debug_shadow"]->setInt("depthMap", 0);
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

	//static void GLAPIENTRY
	//GLerrorCB(GLenum source,
	//	GLenum type,
	//	GLuint id,
	//	GLenum severity,
	//	GLsizei length,
	//	const GLchar* message,
	//	const void* userParam)
	//{
	//	// skip annoying messages
	//	if (severity == 0x33387)
	//		return;
	//	std::cout << "GL CALLBACK: "
	//		<< (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
	//		<< " type = 0x" << type
	//		<< ", severity = 0x" << severity
	//		<< ", message = " << message << std::endl;
	//	//fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	//	//	(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
	//	//	type, severity, message);
	//}

	//constexpr int MAX_BLOCKS = 1;
	//constexpr int MAX_THREADS = 1;
	//constexpr int BUF_OVERWRITE_THRESHOLD = 10000;
	//glm::vec4 colorsVEC[MAX_BLOCKS];
	//glm::mat4 modelsMAT[MAX_BLOCKS];
	////bool cullList[MAX_BLOCKS];
	//float* meshesFLOAT;
	//std::vector<unsigned> updatedBlocks(MAX_BLOCKS);
	//VAO* blockVao;
	//VBO* blockMeshBuffer;
	//VBO* blockModelBuffer;
	//VBO* blockColorBuffer;
	//std::vector<std::thread> threads;

	//void Init()
//{
//	meshesFLOAT = new float[MAX_BLOCKS * _countof(Render::cube_tex_vertices)];
//	glEnable(GL_DEBUG_OUTPUT);
//	glEnable(GL_DEPTH_TEST);

//	// enable debugging stuff
//	glDebugMessageCallback(GLerrorCB, NULL);
//	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
//	glFrontFace(GL_CCW);

//	Shader::shaders["flat"] = new Shader("flat_color_instanced.vs", "flat_color_instanced.fs");
//	Shader::shaders["chunk"] = new Shader("chunk_flat.vs", "chunk_flat.fs");
//	Shader::shaders["chunk_shaded"] = new Shader("chunk_smooth_light.vs", "chunk_smooth_light.fs");
//	Shader::shaders["chunk_shaded"]->Use();
//	Shader::shaders["chunk_shaded"]->setInt("shadowMap", 0);
//	Shader::shaders["sun"] = new Shader("flat_sun.vs", "flat_sun.fs");
//	Shader::shaders["shadow"] = new Shader("shadow.vs", "shadow.fs");
//	Shader::shaders["debug_shadow"] = new Shader("debug_shadow.vs", "debug_shadow.fs");
//	Shader::shaders["debug_shadow"]->Use();
//	Shader::shaders["debug_shadow"]->setInt("depthMap", 0);
//	Shader::shaders["axis"] = new Shader("axis.vs", "axis.fs");
//	Shader::shaders["flat_color"] = new Shader("flat_color.vs", "flat_color.fs");

//	blockVao = new VAO();
//	blockVao->Bind();
//	blockMeshBuffer = new VBO(Render::cube_tex_vertices, sizeof(Render::cube_tex_vertices));

//	blockColorBuffer = new VBO(nullptr, MAX_BLOCKS * sizeof(glm::vec4), GL_DYNAMIC_DRAW);
//	blockModelBuffer = new VBO(nullptr, MAX_BLOCKS * sizeof(glm::mat4), GL_DYNAMIC_DRAW);
//}

//// currently being used to draw everything
//void Draw(LevelPtr level)
//{
//	currShader->Use();
//	currLevel = level;

//	// new draw method (mash every object's data into one vbo/vao and draw that)
//	glm::mat4 view = currCamera->GetView();
//	glm::mat4 proj = currCamera->GetProj();
//	glm::mat4 viewProjection = proj * view;

//	//VBO* vbo = new VBO(Render::cube_tex_vertices, sizeof(Render::cube_tex_vertices));
//	div_t work = div(updatedBlocks.size() , MAX_THREADS);

//	// construct an array of model matrices for OpenGL
//	for (int i = 0; i < MAX_THREADS; i++)
//	{
//		int realWork = work.quot + (i < MAX_THREADS - 1 ? 0 : work.rem);
//		//threads.push_back(std::thread(CalcModels, i * work.quot, i * work.quot + realWork));
//		CalcModels(i * work.quot, i * work.quot + realWork);
//	}

//	for (auto& thread : threads)
//		thread.join();
//	threads.clear();

//	// update model matrix buffer
//	blockModelBuffer->Bind();
//	if (updatedBlocks.size() < BUF_OVERWRITE_THRESHOLD)
//	{
//		for (size_t i = 0; i < updatedBlocks.size(); i++)
//			glBufferSubData(GL_ARRAY_BUFFER, updatedBlocks[i] * sizeof(glm::mat4), sizeof(glm::mat4), &modelsMAT[updatedBlocks[i]]);
//	}
//	else
//	{
//		glBufferData(GL_ARRAY_BUFFER, MAX_BLOCKS * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_BLOCKS * sizeof(glm::mat4), modelsMAT);
//	}
//	
//	// block mesh buffer (constant)
//	blockMeshBuffer->Bind();
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0); // screenpos

//	blockModelBuffer->Bind();
//	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 0));
//	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 4));
//	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 8));
//	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * 12));

//	// update color buffer
//	blockColorBuffer->Bind();

//	if (updatedBlocks.size() < BUF_OVERWRITE_THRESHOLD)
//	{
//		for (size_t i = 0; i < updatedBlocks.size(); i++)
//			glBufferSubData(GL_ARRAY_BUFFER, updatedBlocks[i] * sizeof(glm::vec4), sizeof(glm::vec4), &colorsVEC[updatedBlocks[i]]);
//	}
//	else
//	{
//		glBufferData(GL_ARRAY_BUFFER, MAX_BLOCKS * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_BLOCKS * sizeof(glm::vec4), colorsVEC);
//	}
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

//	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//	glEnableVertexAttribArray(2);
//	glEnableVertexAttribArray(3);
//	glEnableVertexAttribArray(4);
//	glEnableVertexAttribArray(5);
//	glVertexAttribDivisor(0, 0);
//	glVertexAttribDivisor(1, 1);
//	glVertexAttribDivisor(2, 1);
//	glVertexAttribDivisor(3, 1);
//	glVertexAttribDivisor(4, 1);
//	glVertexAttribDivisor(5, 1);

//	currShader->setMat4("u_viewProj", viewProjection);

//	//glDrawArraysInstanced(GL_TRIANGLES, 0, 36, Block::count_);
//	updatedBlocks.clear();
//}
}