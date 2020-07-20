#pragma once
#include <queue>
#include <shared_mutex>
#include <optional>

namespace Net
{
	struct VisiblePlayerState
	{
		glm::vec3 pos{ 0, 0, 0 };
		glm::vec3 front{ 0, 0, 0 };
	};

	struct PlayerObject
	{
		// automagically interpolates state for the user and returns that
		// also handles any possible error case, automagically :)
		VisiblePlayerState GetVisibleState();
		void Update(float dt);

		// buffer containing recent state information about the players
		// back = newest state
		// front = oldest state
		// this queue should not contain more than 2-15 states depending
		// on the server tick rate
		std::deque<VisiblePlayerState> states;

		// constant for interpolating across states
		// 0.0 = front of queue (oldest state)
		// 1.0 = second oldest state, ... etc.
		// 0.5 = 50% of oldest + second oldest state
		// If this value is > 1.0, then the oldest state will be removed
		// and this value will be decremented.
		// THE AMOUNT THIS VALUE WILL BE INCREMENTED EACH FRAME
		// IS (CLIENT DT / SERVER TICK)
		float keyframe = 0.0f;
	};

	// all player objects in the world and recent history associated
	// thread-safe (probably)
	class PlayerWorld
	{
	public:
		// Gets a player object -if it exists- for reading/drawing
		std::optional<PlayerObject> GetObj(int id);
		void RemoveObject(int id);
		void PushState(int id, VisiblePlayerState state);
		void PopState(int id);

		// updates the state of each item 
		void UpdateStates(float dt);

		auto& GetObjects_Unsafe()
		{
			return objects;
		}

	private:
		std::unordered_map<int, PlayerObject> objects;
		std::shared_mutex mtx;
	};
}