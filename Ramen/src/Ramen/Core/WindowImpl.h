#pragma once

#include "IWindow.h"

struct GLFWwindow;

namespace Ramen
{
	class WindowImpl 
		: public IWindow
	{
	public:
		WindowImpl(GLFWwindow* _window);

		// Inherited via IWindow
		virtual vec2 GetWindowSize() override;

	private:
		GLFWwindow* m_window;

		static WindowImpl* s_instance;
		static void WindowResizeCallback(GLFWwindow* _window, int _width, int _height);
	};
}