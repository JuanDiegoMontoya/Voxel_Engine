#pragma once

typedef class Level* LevelPtr;

typedef class Game
{
public:
	Game(GLFWwindow *window);
	~Game();

	inline void SetDimensions(glm::ivec2 dim) { _dimensions = dim; }
	inline glm::ivec2 GetDimensions() { return _dimensions; }

	inline float GetDT() { return _dt; };
	inline float GetTimescale() { return _timescale; }
	inline float SetTimescale(float sc) { _timescale = sc; }

	inline void PauseGame() { _running = false; }
	inline void UnpauseGame() { _running = true; }

	inline GLFWwindow*& GetWindow() { return _window; };

	void Run();
	void Update(float dt);

	// Current level that will be updated and drawn.
	LevelPtr currLevel;

	int score = 0;

private:
	Game() = delete;

	// window context
	GLFWwindow* _window;
	glm::ivec2 _dimensions;

	// temporal
	float _dt = 0.0f;
	float _timescale = 1;

	// pause
	bool _running = true;

}Game, *GamePtr;