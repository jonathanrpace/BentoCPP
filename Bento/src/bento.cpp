#include "bento.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace bento
{
	std::string* Config::s_resourcePath = nullptr;
	std::string* Config::s_defaultsPath = nullptr;

	void Config::Init(std::string _resourcePath, std::string _defaultsPath)
	{
		Shutdown();

		Config::s_resourcePath = new std::string(_resourcePath);
		Config::s_defaultsPath = new std::string(_defaultsPath);
	}

	void Config::Shutdown()
	{
		if (Config::s_resourcePath != nullptr)
			delete s_resourcePath;
		if (Config::s_defaultsPath != nullptr)
			delete s_defaultsPath;
	}

	std::string Config::ResourcePath()
	{
		return *Config::s_resourcePath;
	}

	std::string Config::DefaultsPath()
	{
		return *Config::s_defaultsPath;
	}
}