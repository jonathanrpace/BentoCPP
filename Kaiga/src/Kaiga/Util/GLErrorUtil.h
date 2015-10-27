#pragma once

#include <glew.h>
#include <ramen.h>

#ifdef ENABLE_GL_CHECKS
#	define CHECK_SHADER_COMPILATION(ProgName)								\
	{																		\
		GLint infoLogLength;												\
		glGetProgramiv((ProgName), GL_INFO_LOG_LENGTH, &infoLogLength);		\
		GLchar* infoLog = new GLchar[infoLogLength];						\
		glGetProgramInfoLog((ProgName), infoLogLength, nullptr, infoLog);	\
		if ( infoLogLength > 1 )											\
		{																	\
			TRACE(infoLog);													\
			__debugbreak();													\
		} 																	\
		else																\
		{																	\
			TRACE("Compilation successful\n");								\
		}																	\
		delete infoLog;														\
	}
#else
#	define CHECK_SHADER_COMPILATION(ProgName)
#endif

#ifdef ENABLE_GL_CHECKS

const char* GetOpenGLErrorString(GLenum _error);
void CheckOpenGLError(const char* stmt, const char* fname, int line);

#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0);
#else
#define GL_CHECK(stmt) stmt
#endif