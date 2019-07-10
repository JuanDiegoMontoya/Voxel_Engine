#pragma once

class VAO;
class VBO;

class Sun
{
public:
	Sun();

	inline void Update()
	{
		dir_.x = cos(glfwGetTime());
		dir_.y = sin(glfwGetTime());
		dir_.z = 0;
	}

	void Render();

	inline const glm::vec3& getDir() { return dir_; }

private:
	glm::vec3 dir_;

	VAO* vao_;
	VBO* vbo_;
};