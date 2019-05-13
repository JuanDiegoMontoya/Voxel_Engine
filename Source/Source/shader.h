#pragma once
//#include "stdafx.h"
#include "utilities.h"

// encapsulates shaders by storing uniforms and its GPU memory location
// also stores the program's name and both shader paths for recompiling
typedef class Shader
{
public:
	GLuint programID;		// the program's address in GPU memory
	const int shaderID;	// index into shader array
	std::string name;		// probably actual index into shader array
	std::string vsPath;	// vertex shader path
	std::string fsPath;	// fragment shader path
	std::string cmpPath;// compute shader path

	// maps strings to GPU memory addresses (as GLints)
	std::unordered_map<const char*, GLint, Utils::djb2hash> Uniforms;

	// standard vertex + fragment program constructor
	Shader(const char* vertexPath, const char* fragmentPath);

	// default constructor (currently no uses)
	inline Shader() : shaderID(_shader_count)
	{
		//type = sDefault;
		programID = NULL;
		_shader_count++;
	}

	inline ~Shader()
	{
		glDeleteProgram(programID);
	}

	// set the active shader to this one
	inline void Use()
	{
		glUseProgram(programID);
	}

	inline void setBool(GLint uniformLoc, bool value)
	{
		//glUniform1i(glGetUniformLocation(Program, name), (int)value);
		glUniform1i(uniformLoc, (int)value);
	}
	inline void setInt(GLint uniformLoc, int value)
	{
		glUniform1i(uniformLoc, value);
	}
	inline void setFloat(GLint uniformLoc, float value)
	{
		glUniform1f(uniformLoc, value);
	}
	inline void set3FloatArray(GLint uniformLoc, const float* value, int count)
	{
		glUniform3fv(uniformLoc, count, &value[0]);
	}
	inline void set4FloatArray(GLint uniformLoc, const float* value, int count)
	{
		glUniform4fv(uniformLoc, count, &value[0]);
	}
	inline void setIntArray(GLint uniformLoc, std::vector<int>& value, int count)
	{
		if (value.size() <= 0)
		{
			//traceMessage(std::string("Bad Data - Int vector empty."));
		}
		glUniform1iv(uniformLoc, count, &value[0]);
	}
	inline void set1FloatArray(GLint uniformLoc, std::vector<float>& value, int count)
	{
		if (value.size() <= 0)
		{
			//traceMessage(std::string("Bad Data - Float vector empty."));
		}
		glUniform1fv(uniformLoc, count, &value[0]);
	}
	inline void set2FloatArray(GLint uniformLoc, std::vector<glm::vec2>& value, int count)
	{
		if (value.size() <= 0)
		{
			//traceMessage(std::string("Bad Data - Float vector empty."));
		}
		glUniform2fv(uniformLoc, count, &value[0].x);
	}
	inline void set3FloatArray(GLint uniformLoc, std::vector<glm::vec3>& value, int count)
	{
		if (value.size() <= 0)
		{
			//traceMessage(std::string("Bad Data - Float vector empty."));
		}
		glUniform3fv(uniformLoc, count, &value[0].x);
	}
	inline void set4FloatArray(GLint uniformLoc, std::vector<glm::vec4>& value, int count)
	{
		if (value.size() <= 0)
		{
			//traceMessage(std::string("Bad Data - Float vector empty."));
		}
		glUniform4fv(uniformLoc, count, &value[0].x);
	}
	inline void setVec2(GLint uniformLoc, const glm::vec2 &value)
	{
		glUniform2fv(uniformLoc, 1, &value[0]);
	}
	inline void setVec2(GLint uniformLoc, float x, float y)
	{
		glUniform2f(uniformLoc, x, y);
	}
	inline void setVec3(GLint uniformLoc, const glm::vec3 &value)
	{
		glUniform3fv(uniformLoc, 1, &value[0]);
	}
	inline void setVec3(GLint uniformLoc, float x, float y, float z)
	{
		glUniform3f(uniformLoc, x, y, z);
	}
	inline void setVec4(GLint uniformLoc, const glm::vec4 &value)
	{
		glUniform4fv(uniformLoc, 1, &value[0]);
	}
	inline void setVec4(GLint uniformLoc, float x, float y, float z, float w)
	{
		glUniform4f(uniformLoc, x, y, z, w);
	}
	inline void setMat3(GLint uniformLoc, const glm::mat3 &mat)
	{
		glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, &mat[0][0]);
	}
	inline void setMat4(GLint uniformLoc, const glm::mat4 &mat)
	{
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &mat[0][0]);
	}

	// (static) list of all shader programs
	static std::unordered_map<std::string, Shader*> shaders;
private:
	typedef enum _shadertype : GLint
	{
		TY_VERTEX = GL_VERTEX_SHADER,
		TY_FRAGMENT = GL_FRAGMENT_SHADER,
		TY_COMPUTE = GL_COMPUTE_SHADER
	};

	static int _shader_count;
	static const char* _shader_dir;
	std::string loadShader(const char* path);
	GLint compileShader(_shadertype type, const GLchar* src);
}Shader, *ShaderPtr;