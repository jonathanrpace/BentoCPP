#pragma once

#include <event.h>
#include <glm.h>

namespace Ramen
{
	class IWindow
	{
	public:
		virtual vec2 GetWindowSize() = 0;

		// Events
		DECLARE_EVENT(void, int _width, int _height) OnResize;
	};
}