#include "DefaultsManager.h"

#include <bento/util/FileUtil.h>

namespace bento
{
	std::string DefaultsManager::s_namespace("global");
	json DefaultsManager::s_data;

	void bento::DefaultsManager::Init(std::string _filepath)
	{
		char* defaultsFileContents;
		unsigned long defaultsFileContentsLength;

		int error = fileUtil::LoadFile(_filepath.c_str(), &defaultsFileContents, &defaultsFileContentsLength);

		// File doesn't exist
		if (error == -1)
		{
			s_data.clear();
			return;
		}

		s_data = json::parse(defaultsFileContents);

		delete[] defaultsFileContents;

	}

	void DefaultsManager::Shutdown()
	{
		s_data.clear();
		s_namespace.clear();
	}

	void DefaultsManager::SetNamespace(std::string _namespace)
	{
		s_namespace = _namespace;
	}

}