#include "FixedPhysics.h"

void PhysicsWorld::Init()
{
	thread = std::make_unique<std::thread>(run);
}


void PhysicsWorld::Shutdown()
{
	shutdownThreads = true;
	thread->join();
}


void PhysicsWorld::run()
{
	Timer timer;
	double accum = 0;
	// accumulate time
	while (!shutdownThreads)
	{
		accum += timer.elapsed();
		timer.reset();

		while (accum > freq)
		{
			accum -= freq;

			for (const auto& obj : objects)
			{
				obj->Update();
			}
		}
	}
}


void PhysicsWorld::cleanup()
{

}
