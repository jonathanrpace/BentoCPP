#include "WindowImpl.h"

#include <glew.h>
#include <glfw3.h>
#include <assert.h>

Ramen::WindowImpl* Ramen::WindowImpl::s_instance = nullptr;

//////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////

Ramen::WindowImpl::WindowImpl(GLFWwindow* _window) :
	m_window(_window)
{

}

vec2 Ramen::WindowImpl::GetWindowSize()
{
	int width;
	int height;
	glfwGetWindowSize(m_window, &width, &height);
	return vec2((float)width, (float)height);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void Ramen::WindowImpl::WindowResizeCallback(GLFWwindow * _window, int _width, int _height)
{
	TRIGGER_EVENT(s_instance->OnResize, _width, _height);
}
