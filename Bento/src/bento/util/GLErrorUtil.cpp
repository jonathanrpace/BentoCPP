#include "GLErrorUtil.h"

#include <bento.h>

const char * GetOpenGLErrorString(GLenum _error)
{
	switch (_error)
	{
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		case GL_OUT_OF_MEMORY:
			return "GL_OUT_OF_MEMORY";
		case GL_STACK_UNDERFLOW:
			return "GL_STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:
			return "GL_STACK_OVERFLOW";
	}

	return "Unknown error";
}

void CheckOpenGLError(const char * stmt, const char * fname, int line)
{
	GLenum err = glGetError();
	bool foundError = false;
	while (err != GL_NO_ERROR)
	{
		PRINTF("ERROR: %s\n  %s\n  %s:%i\n",
			GetOpenGLErrorString(err), stmt, fname, line );
		foundError = true;
		err = glGetError();
	}

	if (foundError)
	{
		__debugbreak();
	}
}