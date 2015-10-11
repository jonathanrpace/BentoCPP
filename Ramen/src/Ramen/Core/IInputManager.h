#pragma once

#include <event.h>
#include <glm.h>

namespace Ramen
{
	class IInputManager
	{
	public:
		virtual vec2 GetMousePosition() = 0;

		// Events
		DECLARE_EVENT(void, int _button) OnMouseButtonPress;
		DECLARE_EVENT(void, int _button) OnMouseButtonRelease;
		DECLARE_EVENT(void, vec2 _pos) OnMouseMove;
	};
}
