#pragma once

#include <event.h>
#include <glm.h>

namespace bento
{
	class IWindow
	{
	public:
		virtual ivec2 GetWindowSize() = 0;

		// Events
		DECLARE_EVENT(void, int _width, int _height) OnResize;
	};
}