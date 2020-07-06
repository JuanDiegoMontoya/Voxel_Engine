#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include "Timer.h"

struct PhysicsObject
{
	virtual void Update();
};


struct PlayerPhysics : public PhysicsObject
{

};


// fixed-tick player physics simulation
class PhysicsWorld
{
public:
	// spawns a thread running the physics simulation
	static void Init();

	static void Shutdown();

	static void PushObject(std::unique_ptr<PhysicsObject> obj);

	// time between physics updates
	static inline double freq = 0.1; // 1.0 / freq = hz

private:
	static void run(); // run in another thread
	static void cleanup();

	static inline std::unique_ptr<std::thread> thread;
	static inline bool shutdownThreads = false;

	static inline std::vector<std::unique_ptr<PhysicsObject>> objects;
};