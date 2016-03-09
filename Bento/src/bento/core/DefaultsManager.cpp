#include "DefaultsManager.h"

#include <bento/util/FileUtil.h>

namespace bento
{
	std::string* DefaultsManager::s_namespaceName;
	std::string* DefaultsManager::s_filepath;
	json* DefaultsManager::s_data;
	json DefaultsManager::s_namespace;

	void bento::DefaultsManager::Init(std::string _filepath)
	{
		s_filepath = new std::string(_filepath);
		s_namespace = json::object();
		s_namespaceName = new std::string("global");
		

		SetNamespace("global");

		char* defaultsFileContents;
		unsigned long defaultsFileContentsLength;

		int error = fileUtil::LoadFile((*s_filepath).c_str(), &defaultsFileContents, &defaultsFileContentsLength);

		// File doesn't exist
		if (error == -1)
		{
			s_data = json::object();
			return;
		}

		auto tmp = json::parse(defaultsFileContents);
		s_data = (json*)tmp;
		delete[] defaultsFileContents;
	}

	void DefaultsManager::Flush()
	{
		SetNamespace("global");
		std::string str = s_data.dump(4);
		fileUtil::SaveFile((*s_filepath).c_str(), str.c_str(), str.size());
	}

	void DefaultsManager::Shutdown()
	{
		s_data.clear();
		s_namespace.clear();
		delete s_filepath;
		delete s_namespaceName;
	}

	void DefaultsManager::SetNamespace(std::string _namespace)
	{
		// Before switching namespace, ensure the old one
		// gets stashed
		if (s_namespace.is_null() == false)
		{
			s_data[(*s_namespaceName).c_str()] = s_namespace;
		}

		delete s_namespaceName;
		s_namespaceName = new std::string(_namespace);

		s_namespace = s_data[(*s_namespaceName).c_str()];
		if (s_namespace.is_null())
		{
			s_data[(*s_namespaceName).c_str()] = json::object();
		}
	}

	void DefaultsManager::GetValue(const char* _key, float _default, float* o_value)
	{
		bool isNull = s_namespace[_key].is_null();
		if (isNull)
		{
			s_namespace[_key] = _default;
			(*o_value) = _default;
			return;
		}
		float value = s_namespace[_key].get<float>();
		(*o_value) = value;
	}

}