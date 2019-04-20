#include "stdafx.h"
#include "shader.h"
#include <fstream>

int Shader::_shader_count = 0;

//Shader::Shader(const char* vertexPath, const char* fragmentPath)
//{
//	//type = t;
//
//	vertexPath = path + vertexPath;
//	fragmentPath = path + fragmentPath;
//
//	std::string vertexCode;
//	std::string fragmentCode;
//
//	std::ifstream vertexFile;
//	std::ifstream fragmentFile;
//
//	std::stringstream vertexStream;
//	std::stringstream fragmentStream;
//
//	vertexFile.exceptions(std::ifstream::badbit);
//	fragmentFile.exceptions(std::ifstream::badbit);
//
//	try
//	{
//		vertexFile.open(vertexPath);
//		fragmentFile.open(fragmentPath);
//
//		vertexStream << vertexFile.rdbuf();
//		fragmentStream << fragmentFile.rdbuf();
//
//		vertexFile.close();
//		fragmentFile.close();
//
//		vertexCode = vertexStream.str();
//		fragmentCode = fragmentStream.str();
//
//		traceMessage("\nShader: " + vertexPath + ", " + fragmentPath + " successfully opened for reading");
//
//	}
//	catch (std::ifstream::failure e)
//	{
//		std::cout << "Files: " << vertexPath << ", " << fragmentPath << std::endl;
//		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << e.what() << std::endl;
//		traceMessage(std::string(e.what()));
//	}
//
//	const GLchar* vShaderCode = vertexCode.c_str();
//	const GLchar* fShaderCode = fragmentCode.c_str();
//
//	GLuint vertex, fragment;
//	GLint success;
//	GLchar infoLog[512];
//
//	vertex = glCreateShader(GL_VERTEX_SHADER);
//
//	glShaderSource(vertex, 1, &vShaderCode, NULL);
//	glCompileShader(vertex);
//
//	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
//		std::cout << "File: " << vertexPath << std::endl;
//		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//		traceMessage(std::string(infoLog));
//	}
//	else
//	{
//		traceMessage("Vertex Shader good compile");
//	}
//
//	fragment = glCreateShader(GL_FRAGMENT_SHADER);
//
//	glShaderSource(fragment, 1, &fShaderCode, NULL);
//	glCompileShader(fragment);
//
//	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
//		std::cout << "File: " << fragmentPath << std::endl;
//		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//		traceMessage(std::string(infoLog));
//	}
//	else
//	{
//		traceMessage("Fragment Shader good compile");
//	}
//
//	// Link shaders
//	Program = glCreateProgram();
//
//	glAttachShader(Program, vertex);
//	glAttachShader(Program, fragment);
//	glLinkProgram(Program);
//
//	// Check for linking errors
//	glGetProgramiv(Program, GL_LINK_STATUS, &success);
//	if (!success)
//	{
//		glGetProgramInfoLog(Program, 512, NULL, infoLog);
//		std::cout << "Files: " << vertexPath << ", " << fragmentPath << std::endl;
//		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//		traceMessage(std::string(infoLog));
//	}
//	else
//	{
//		traceMessage("Shader good link");
//	}
//
//	glDeleteShader(vertex);
//	glDeleteShader(fragment);
//
//	// Setup Uniform Map
//	{
//		GLint max_length;
//		glGetProgramiv(Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
//		GLint num_uniforms;
//
//		GLchar* pname = new GLchar[max_length];
//
//		glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &num_uniforms);
//
//		for (GLint i = 0; i < num_uniforms; ++i)
//		{
//			GLsizei written;
//			GLint size;
//			GLenum type;
//
//			glGetActiveUniform(Program, i, max_length, &written, &size, &type, pname);
//
//			GLchar* pname1 = new GLchar[max_length];
//
//			std::strcpy(pname1, pname);
//
//			if (size > 1)
//			{
//				pname1[written - 3] = '\0';
//			}
//
//			GLint loc = glGetUniformLocation(Program, pname1);
//
//			Uniforms.insert(std::pair<GLchar*, GLint>(pname1, loc));
//		}
//
//		delete[] pname;
//	}
//}