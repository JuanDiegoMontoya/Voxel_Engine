#pragma once
#include "game.h"
#include "game_object.h"
#include "camera.h"

typedef class Level
{
public:
	Level(std::string name); // will load from file in the future (maybe)
	~Level();
	void Init();
	void Update(float dt);
	void CheckCollision();
	void CheckInteraction();

	inline GamePtr Game() { return _game; }
	inline const std::vector<GameObjectPtr>& GetObjects() const { return _objects; }

private:
	std::string _name; // name of file
	GamePtr _game;
	std::vector<Camera*> _cameras;			 // all cameras in the scene
	std::vector<GameObjectPtr> _objects; // all game objects in the scene

}Level, *LevelPtr;