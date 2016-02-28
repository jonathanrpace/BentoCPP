#pragma once

#include <bento.h>

namespace bento
{
	struct IWindow
	{
		virtual ivec2 GetWindowSize() = 0;

		// Events
		DECLARE_EVENT(void, int _width, int _height) OnResize;
	};
}