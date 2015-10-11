#include "InputManagerImpl.h"

#include <glew.h>
#include <glfw3.h>
#include <assert.h>

Ramen::InputManagerImpl* Ramen::InputManagerImpl::s_instance = nullptr;

//////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////

Ramen::InputManagerImpl::InputManagerImpl(GLFWwindow* _window) :
	m_window(_window)
{
	assert(s_instance == nullptr);
	s_instance = this;

	glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
	glfwSetCursorPosCallback(m_window, MouseMoveCallback);
}

Ramen::InputManagerImpl::~InputManagerImpl()
{
	glfwSetMouseButtonCallback(m_window, nullptr);
	glfwSetCursorPosCallback(m_window, nullptr);
}

vec2 Ramen::InputManagerImpl::GetMousePosition()
{
	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	return vec2((float)x, (float)y);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void Ramen::InputManagerImpl::MouseButtonCallback(GLFWwindow * _window, int _button, int _action, int _mods)
{
	if ( _action == GLFW_PRESS )
	{
		TRIGGER_EVENT(s_instance->OnMouseButtonPress, _button);
	}
	else if ( _action == GLFW_RELEASE )
	{
		TRIGGER_EVENT(s_instance->OnMouseButtonRelease, _button);
	}
}

void Ramen::InputManagerImpl::MouseMoveCallback(GLFWwindow * window, double _x, double _y)
{
	TRIGGER_EVENT(s_instance->OnMouseMove, vec2((float)_x, (float)_y));
}
