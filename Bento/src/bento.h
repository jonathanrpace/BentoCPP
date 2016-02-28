#pragma once

#include <windows.h>
#include <assert.h>
#include <vector>
#include <memory>
#include <typeinfo>
#include <string.h>

#include <gl/glew.h>
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
#include <bento/core/event.h>
#include <bento/util/GLErrorUtil.h>

#define DEG_TO_RAD 0.01745329251994329576923690768489f
#define RAD_TO_DEG 57.295779513082320876798154814105f



namespace bento
{
	struct Config
	{
	public:
		static void Init
		(
			std::string _resourcePath,
			std::string _defaultsPath
		);

		static void Shutdown();

		static std::string ResourcePath();
		static std::string DefaultsPath();

	private:
		static std::string* s_resourcePath;
		static std::string* s_defaultsPath;
	};
}