#pragma once
#include <varargs.h>
#include <queue>
#include <shared_mutex>

#define REGISTER_EVENT(x) x* x ## _data
#define PROCESS_EVENT(x) case Packet::e ## x: data.x ## _data->Process(); break
#define UNDEFINED_EVENT(x) ;

namespace Net
{
	struct Packet
	{
		enum EventType { eNullEvent, ePrintCameraPosEvent, eInputEvent };

		int type;
		union
		{
			REGISTER_EVENT(PrintCameraPosEvent);
			REGISTER_EVENT(InputEvent);
			//PrintCameraPosEvent* a;
			//InputEvent* b;
		}data;
	};

	struct PrintCameraPosEvent
	{
		void Process()
		{
			printf("(%f, %f, %f)\n", pos.x, pos.y, pos.z);
		}
		glm::vec3 pos;
	};

	struct InputEvent
	{
		void Process()
		{
			printf("%d\n", input);
		}
		int input;
	};

	struct EventController
	{
	public:
		static void PushEvent(Packet&& p)
		{
			mtx.lock();
			backBuffer->push(std::move(p));
			mtx.unlock();
		}

		static Packet PopEvent()
		{
			mtx.lock();
			Packet&& t = std::move(packetBuf.front());
			packetBuf.pop();
			mtx.unlock();
		}

		static void SwapBuffers()
		{
			mtx.lock();
			packetBuf.swap(backBuffer);
			mtx.unlock();
		}

	private:
		static std::queue<Packet> packetBuf;
		static std::queue<Packet> backBuffer;
		static std::shared_mutex mtx;
	};
}