#pragma once

#include <event.h>
#include <bento.h>

namespace bento
{
	class IInputManager
	{
	public:
		virtual vec2 GetMousePosition() = 0;
		virtual bool IsMouseDown(int _button) = 0;
		virtual bool IsKeyDown(int _key) = 0;

		// Events
		DECLARE_EVENT(void, int _button) OnMouseButtonPress;
		DECLARE_EVENT(void, int _button) OnMouseButtonRelease;
		DECLARE_EVENT(void, vec2 _pos) OnMouseMove;

		DECLARE_EVENT(void, int _key) OnKeyPress;
		DECLARE_EVENT(void, int _jey) OnKeyRelease;
	};
}
