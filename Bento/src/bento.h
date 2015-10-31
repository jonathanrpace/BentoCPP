#pragma once

#include <windows.h>
#include <assert.h>

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