#include "stdafx.h"
#include "shader.h"
#include <sstream>
#include <fstream>

// static class variable definitions
int Shader::shader_count_ = 0;
const char* Shader::shader_dir_ = "./resources/Shaders/";
std::unordered_map<std::string, Shader*> Shader::shaders = std::unordered_map<std::string, Shader*>();

// the provided path does not need to include the shader directory
Shader::Shader(const char* vertexPath, const char* fragmentPath) : shaderID(shader_count_++)
{
	const std::string vertSrc = loadShader(vertexPath).c_str();
	const std::string fragSrc = loadShader(fragmentPath).c_str();

	GLint success;
	GLchar infoLog[512];

	// compile individual shaders
	programID = glCreateProgram();
	GLint vShader = compileShader(TY_VERTEX, vertSrc.c_str());
	GLint fShader = compileShader(TY_FRAGMENT, fragSrc.c_str());
	glAttachShader(programID, vShader);
	glAttachShader(programID, fShader);
	glLinkProgram(programID);

	// link program
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cout << "Files: " << vertexPath << ", " << fragmentPath << '\n';
		std::cout << "Failed to link shader program\n" << infoLog << std::endl;
		//traceMessage(std::string(infoLog));
	}
	else
	{
		//traceMessage("Shader good link");
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	// init uniform map used in that shader
	{
		GLint max_length;
		GLint num_uniforms;

		glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
		//GLchar* pname = (GLchar*)alloca(max_length * sizeof(GLchar));
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
				pname1[written - 3] = '\0';
			GLint loc = glGetUniformLocation(programID, pname1);
			Uniforms.insert(std::pair<GLchar*, GLint>(pname1, loc));
		}

		// unfortunately we must have this in the same scope as where it's constructed
		// alloca prevents the use of delete, but its implementation is compiler dependent
		delete[] pname;
	}
}

// loads a shader source into a string
std::string Shader::loadShader(const char* path)
{
	std::string shaderpath = std::string(shader_dir_) + path;
	std::string content;
	try
	{
		std::ifstream ifs(shaderpath);
		content = std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		//traceMessage("\nShader: " + vertexPath + ", " + fragmentPath + " successfully opened for reading");
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Error reading shader: " << path << '\n';
		std::cout << "Message: " << e.what() << std::endl;
		//traceMessage(std::string(e.what()));
	}
	return std::string(content);
}

// compiles a shader source and returns its ID
GLint Shader::compileShader(shadertype type, const GLchar* src)
{
	GLuint shader = 1001; // init to debug value
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
		path = nullptr;
		break;
	}

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success && path)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "File: " << *path << std::endl;
		std::cout << "Error compiling shader of type " << type << '\n' << infoLog << std::endl;
		//traceMessage(std::string(infoLog));
	}
	else
	{
		//traceMessage("Vertex Shader good compile");
	}

	return shader;
}
