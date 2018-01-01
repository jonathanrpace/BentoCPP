#pragma once

#include <bento.h>
#include <bento/core/IInputManager.h>

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

		virtual bool IsKeyDown(int _key);
		
	private:
		GLFWwindow* m_window;

		static InputManagerImpl* s_instance;
		static void MouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
		static void MouseMoveCallback(GLFWwindow* window, double _x, double _y);
		static void KeyCallback(GLFWwindow* window, int _key, int _scanCode, int _action, int _mods);
		static void ScrollCallback(GLFWwindow* window, double _xoffset, double _yoffset);
	};
}