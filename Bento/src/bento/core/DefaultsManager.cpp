#include "DefaultsManager.h"

#include <assert.h>
#include <bento/util/FileUtil.h>

namespace bento
{
	////////////////////////////////////////////////////////////////////
	// STATIC
	////////////////////////////////////////////////////////////////////

	DefaultsManager* DefaultsManager::s_impl(nullptr);

	void DefaultsManager::Init(std::string _filepath)
	{
		assert(s_impl == nullptr);
		s_impl = new DefaultsManager(_filepath);
	}

	void DefaultsManager::Shutdown()
	{
		if (s_impl != nullptr) {
			delete s_impl;
		}
	}

	void DefaultsManager::SetNamespace(std::string _namespace)
	{
		return s_impl->_SetNamespace(_namespace);
	}

	void DefaultsManager::Flush()
	{
		return s_impl->_Flush();
	}

	////////////////////////////////////////////////////////////////////
	// Private implementation
	////////////////////////////////////////////////////////////////////

	DefaultsManager::DefaultsManager(std::string _filepath)
		: m_filepath()
		, m_namespaceName()
		, m_data()
		, m_namespace()
	{
		m_filepath = _filepath;

		char* defaultsFileContents;
		unsigned long defaultsFileContentsLength;

		int error = fileUtil::LoadFile(m_filepath.c_str(), &defaultsFileContents, &defaultsFileContentsLength);

		// File doesn't exist
		if (error == -1)
		{
			m_data.clear();
			_SetNamespace("global");
			return;
		}

		m_data = json::parse(defaultsFileContents);
		delete[] defaultsFileContents;
	}

	DefaultsManager::~DefaultsManager()
	{
		m_data.clear();
		m_namespace.clear();
	}

	void DefaultsManager::_Flush()
	{
		_SetNamespace("global");
		std::string str = m_data.dump(4);
		fileUtil::SaveFile(m_filepath.c_str(), str.c_str(), str.size());
	}

	void DefaultsManager::_SetNamespace(std::string _namespace)
	{
		// Before switching namespace, ensure the old one
		// gets stashed
		if (m_namespace.is_null() == false)
		{
			m_data[m_namespaceName.c_str()] = m_namespace;
		}

		m_namespaceName = std::string(_namespace);

		m_namespace = m_data[m_namespaceName.c_str()];
		if (m_namespace.is_null())
		{
			m_namespace = json::object();
		}
	}

}