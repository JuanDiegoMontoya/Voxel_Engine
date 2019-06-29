#pragma once

typedef class Level* LevelPtr;

typedef class Game
{
public:
	Game(GLFWwindow *window);
	~Game();

	inline void SetDimensions(glm::ivec2 dim) { dimensions_ = dim; }
	inline glm::ivec2 GetDimensions() { return dimensions_; }

	inline float GetDT() { return dt_; };
	inline float GetTimescale() { return timescale_; }
	inline float SetTimescale(float sc) { timescale_ = sc; }

	inline void PauseGame() { running_ = false; }
	inline void UnpauseGame() { running_ = true; }

	inline GLFWwindow*& GetWindow() { return window_; };

	void Run();
	void Update(float dt);

	// Current level that will be updated and drawn.
	LevelPtr currLevel;

	int score = 0;

private:
	Game() = delete;

	// window context
	GLFWwindow* window_;
	glm::ivec2 dimensions_;

	// temporal
	float dt_ = 0.0f;
	float timescale_ = 1;

	// pause
	bool running_ = true;

}Game, *GamePtr;