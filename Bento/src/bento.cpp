#include "bento.h"

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