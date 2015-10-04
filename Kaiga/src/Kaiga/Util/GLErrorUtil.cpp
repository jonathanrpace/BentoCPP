#include "GLErrorUtil.h"

#ifdef ENABLE_GL_CHECKS
const char * Kaiga::GetOpenGLErrorString(GLenum _error)
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

void Kaiga::CheckOpenGLError(const char * stmt, const char * fname, int line)
{
	GLenum err = glGetError();
	bool foundError = false;
	while (err != GL_NO_ERROR)
	{
		printf("OpenGL error %s, at %s:%i - for %s\n",
			GetOpenGLErrorString(err), fname, line, stmt);
		foundError = true;
		err = glGetError();
	}

	if (foundError)
	{
		abort();
	}
}
#endif