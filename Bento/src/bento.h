#pragma once

#include <windows.h>
#include <assert.h>
#include <vector>
#include <memory>
#include <typeinfo>

#include <glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat4 mat4;
typedef glm::mat3 mat3;

#include <bento/core/Logging.h>
#include <bento/util/GLErrorUtil.h>

namespace bento
{
	struct Config
	{
	public:
		static char* ResourcePath();
		static void ResourcePath(char* _resourcePath);
	private:
		static char* s_resourcePath;
	};
}