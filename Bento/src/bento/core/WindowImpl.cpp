#include "WindowImpl.h"

#include <gl/glew.h>
#include <glfw3.h>
#include <assert.h>

bento::WindowImpl* bento::WindowImpl::s_instance = nullptr;

//////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////

bento::WindowImpl::WindowImpl(GLFWwindow* _window) :
	m_window(_window)
{

}

ivec2 bento::WindowImpl::GetWindowSize()
{
	int width;
	int height;
	glfwGetWindowSize(m_window, &width, &height);
	return ivec2(width, height);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void bento::WindowImpl::WindowResizeCallback(GLFWwindow * _window, int _width, int _height)
{
	TRIGGER_EVENT(s_instance->OnResize, _width, _height);
}
