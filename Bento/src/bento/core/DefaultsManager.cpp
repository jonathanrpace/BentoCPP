#include "DefaultsManager.h"

#include <bento/util/FileUtil.h>

namespace bento
{
	std::string* DefaultsManager::s_namespace;
	std::string* DefaultsManager::s_filepath;
	json* DefaultsManager::s_data;

	void bento::DefaultsManager::Init(std::string _filepath)
	{
		s_filepath = new std::string(_filepath);
		s_namespace = new std::string("global");
		s_data = new json();

		char* defaultsFileContents;
		unsigned long defaultsFileContentsLength;

		int error = fileUtil::LoadFile((*s_filepath).c_str(), &defaultsFileContents, &defaultsFileContentsLength);

		// File doesn't exist
		if (error == -1)
		{
			(*s_data).clear();
			return;
		}

		s_data = &json::parse(defaultsFileContents);
		delete[] defaultsFileContents;
	}

	void DefaultsManager::Flush()
	{
		std::string str = (*s_data).dump();
		fileUtil::SaveFile((*s_filepath).c_str(), str.c_str(), str.size());
	}

	void DefaultsManager::Shutdown()
	{
		delete s_data;
		delete s_filepath;
		delete s_namespace;
	}

	void DefaultsManager::SetNamespace(std::string _namespace)
	{
		delete s_namespace;
		s_namespace = new std::string(_namespace);

		if ((*s_data)[(*s_namespace).c_str()].is_null())
		{
			(*s_data)[(*s_namespace).c_str()] = json::object();
		}
	}

}