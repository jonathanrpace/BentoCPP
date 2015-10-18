#include <ramen.h>

namespace Ramen
{
	char* s_resourcePath = "./resources/";

	char* ResourcePath()
	{
		return s_resourcePath;
	}

	void ResourcePath(char* _resourcePath)
	{
		Ramen::s_resourcePath = _resourcePath;
	}
}