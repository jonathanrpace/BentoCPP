#pragma once

#include <bento.h>
#include <bento/core/IInputManager.h>

#include <event.h>

struct GLFWwindow;

namespace bento
{
	class InputManagerImpl : public IInputManager
	{
	public:
		InputManagerImpl(GLFWwindow* _window);
		~InputManagerImpl();

		virtual vec2 GetMousePosition() override;
		virtual bool IsMouseDown(int _button);
		
	private:
		GLFWwindow* m_window;

		static InputManagerImpl* s_instance;
		static void MouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
		static void MouseMoveCallback(GLFWwindow* window, double _x, double _y);
	};
}