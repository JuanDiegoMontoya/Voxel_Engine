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

// graphics pipeline: the order in which various "things" will be rendered

static void GLAPIENTRY
GLerrorCB(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	std::cout << "GL CALLBACK: "
		<< (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
		<< " type = 0x" << type
		<< ", severity = 0x" << severity
		<< ", message = " << message << std::endl;
	//fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	//	(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
	//	type, severity, message);
}

namespace Render
{
	std::unordered_map<const char*, Texture*, Utils::djb2hash, Utils::charPtrKeyEq> textures;
	Camera* currCamera;
	ShaderPtr currShader;
	Renderer renderer;

	VBO* vbo;
	IBO* ibo;
	VAO* vao;
	Texture* texture;
	
	// x, y, z (mins and maxes)
	glm::mat4 proj;
	glm::mat4 view;
	glm::mat4 model;
	glm::mat4 mvp;

	glm::vec3 translationA = glm::vec3(200, 200, 0);
	glm::vec3 translationB = glm::vec3(400, 200, 0);
	glm::vec3 camerapos = glm::vec3(0, 0, 0);

	void Init()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GLerrorCB, NULL);
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		currShader = Shader::shaders["flat"] = new Shader("flat_color.vs", "flat_color.fs");

		/*
		float positions[] =
		{
			-50.f, -50.f, 0.0f, 0.0f,
			 50.f, -50.f, 1.0f, 0.0f,
			 50.f,  50.f, 1.0f, 1.0f,
			-50.f,  50.f, 0.0f, 1.0f
		};

		GLuint indices[] =
		{
			0, 1, 2,
			2, 3, 0
		};


		// vertex array + buffer
		vao = new VAO();
		vbo = new VBO(positions, sizeof(positions));
		VBOlayout layout;
		layout.Push<GLfloat>(2);
		layout.Push<GLfloat>(2);
		vao->AddBuffer(*vbo, layout);
		vbo->Unbind();

		// index buffer
		ibo = new IBO(indices, 6);

		currShader = Shader::shaders["test"];
		currShader->setInt("u_texture", 0);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		texture = new Texture("deet.png");
		texture->Bind();

		vao->Unbind();

		proj = glm::ortho(0.f, 1920.f, 0.f, 1080.f, -1.f, 1.f);
		*/
	}

	// currently being used to draw everything
	void Draw(LevelPtr level)
	{
		for (auto& obj : level->GetObjects())
		{
			if (obj->GetEnabled())
			{
				RenderDataPtr rend = obj->GetComponent<RenderData>();
				if (rend && rend->GetEnabled())
				{
					currShader = rend->GetShader();
					currShader->Use();
					currShader->setVec4("u_color", glm::vec4(1.f));
					currShader->setMat4("u_model", obj->GetComponent<Transform>()->GetModel());
					currShader->setMat4("u_view", currCamera->GetView());
					currShader->setMat4("u_proj", currCamera->GetProj());

					renderer.DrawArrays(rend->GetVao(), 36, *currShader);
				}
			}
		}

		/*
		renderer.Clear();
		currShader->Use();

		view = glm::translate(glm::mat4(1.0f), camerapos);

		{ // obj A
			model = glm::translate(glm::mat4(1.0f), translationA);
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0, 0, 1.f));
			mvp = proj * view * model;
			currShader->setMat4("u_mvp", mvp);
			renderer.Draw(*vao, *ibo, *currShader);
		}

		{ // obj B
			model = glm::translate(glm::mat4(1.0f), translationB);
			mvp = proj * view * model;
			currShader->setMat4("u_mvp", mvp);
			renderer.Draw(*vao, *ibo, *currShader);
		}
		*/
	}

	void SetCamera(Camera* cam)
	{
		currCamera = cam;
	}

	void drawImGui()
	{
		ImGui::Begin("Giraffix");

		ImGui::SliderFloat3("TranslationA", &translationA.x, 0, 1000);
		ImGui::SliderFloat3("TranslationB", &translationB.x, 0, 1000);
		ImGui::SliderFloat3("Camera Pos", &camerapos.x, -500, 500);

		ImGui::End();
	}

	void Terminate()
	{

	}
}