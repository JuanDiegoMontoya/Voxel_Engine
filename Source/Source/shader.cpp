#include "stdafx.h"
#include "shader.h"
#include <sstream>
#include <fstream>

int Shader::_shader_count = 0;
const char* Shader::_shader_dir = "./resources";

Shader::Shader(const char* vertexPath, const char* fragmentPath) : shaderID(_shader_count++)
{
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	GLint success;
	GLchar infoLog[512];

	// Link shaders
	programID = glCreateProgram();

	glAttachShader(programID, compileShader(TY_VERTEX, vShaderCode));
	glAttachShader(programID, compileShader(TY_FRAGMENT, fShaderCode));
	glLinkProgram(programID);

	// Check for linking errors
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cout << "Files: " << vertexPath << ", " << fragmentPath << std::endl;
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		//traceMessage(std::string(infoLog));
	}
	else
	{
		//traceMessage("Shader good link");
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	// Setup Uniform Map
	{
		GLint max_length;
		glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
		GLint num_uniforms;

		GLchar* pname = new GLchar[max_length];

		glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &num_uniforms);

		for (GLint i = 0; i < num_uniforms; ++i)
		{
			GLsizei written;
			GLint size;
			GLenum type;

			glGetActiveUniform(programID, i, max_length, &written, &size, &type, pname);

			GLchar* pname1 = new GLchar[max_length];

			std::strcpy(pname1, pname);

			if (size > 1)
			{
				pname1[written - 3] = '\0';
			}

			GLint loc = glGetUniformLocation(programID, pname1);

			Uniforms.insert(std::pair<GLchar*, GLint>(pname1, loc));
		}

		delete[] pname;
	}
}

void Shader::loadShader(const char* path)
{
	std::string vpath = (std::string)_shader_dir + vertexPath;
	std::string fpath = (std::string)_shader_dir + fragmentPath;

	std::string vertexCode;
	std::string fragmentCode;

	std::ifstream vertexFile;
	std::ifstream fragmentFile;

	std::stringstream vertexStream;
	std::stringstream fragmentStream;

	vertexFile.exceptions(std::ifstream::badbit);
	fragmentFile.exceptions(std::ifstream::badbit);

	//std::ifstream ifs(script.name.c_str());
	//std::string content((std::istreambuf_iterator<char>(ifs)),
	//	(std::istreambuf_iterator<char>()));

	try
	{
		vertexFile.open(vertexPath);
		fragmentFile.open(fragmentPath);

		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();

		vertexFile.close();
		fragmentFile.close();

		vertexCode = vertexStream.str();
		fragmentCode = fragmentStream.str();

		//traceMessage("\nShader: " + vertexPath + ", " + fragmentPath + " successfully opened for reading");

	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Files: " << vertexPath << ", " << fragmentPath << std::endl;
		std::cout << "ERROR: FAILED TO READ SHADER" << e.what() << std::endl;
		//traceMessage(std::string(e.what()));
	}
}

GLint Shader::compileShader(_shadertype type, const GLchar* src)
{
	GLuint shader;
	GLchar infoLog[512];
	std::string* path;
	GLint success;
	
	switch (type)
	{
	case Shader::TY_VERTEX:
		shader = glCreateShader(GL_VERTEX_SHADER);
		path = &vsPath;
		break;
	case Shader::TY_FRAGMENT:
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		path = &fsPath;
		break;
	case Shader::TY_COMPUTE:
		shader = glCreateShader(GL_COMPUTE_SHADER);
		path = &cmpPath;
		break;
	default:
		//traceMessage("Unknown shader type");
		break;
	}

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "File: " << *path << std::endl;
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILURE\n" << infoLog << std::endl;
		//traceMessage(std::string(infoLog));
	}
	else
	{
		//traceMessage("Vertex Shader good compile");
	}

	return shader;
}
