#pragma once
#include <varargs.h>

#define REGISTER_EVENT(x) x* x ## _data
#define PROCESS_EVENT(x) case Packet::e ## x: data.x ## _data->Process(); break
#define UNDEFINED_EVENT(x) ;

namespace Net
{
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
}