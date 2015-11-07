#include "bento.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace bento
{
	char* Config::s_resourcePath = "./resources/";

	char* Config::ResourcePath()
	{
		return Config::s_resourcePath;
	}

	void Config::ResourcePath(char* _resourcePath)
	{
		Config::s_resourcePath = _resourcePath;
	}
}