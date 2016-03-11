#include "SerializableBase.h"

#include <bento/core/DefaultsManager.h>

namespace bento
{
	SerializableBase::SerializableBase(char * const _namespace)
		: m_namespace(_namespace)
		, m_names(64)
		, m_types(64)
		, m_numMembers(0)
		, m_floatDefaults(64)
		, m_floatPointers(64)
		, m_intDefaults(64)
		, m_intPointers(64)
	{
	}

	void SerializableBase::SerializableMember(char const * _name, float _default, float* _ptr)
	{
		m_names[m_numMembers] = _name;
		m_types[m_numMembers] = eSerializableType_float;
		m_floatDefaults[m_numMembers] = _default;
		m_floatPointers[m_numMembers] = _ptr;
		m_numMembers++;
	}

	void SerializableBase::SerializableMember(char const * _name, int _default, int* _ptr)
	{
		m_names[m_numMembers] = _name;
		m_types[m_numMembers] = eSerializableType_int;
		m_intDefaults[m_numMembers] = _default;
		m_intPointers[m_numMembers] = _ptr;
		m_numMembers++;
	}

	void SerializableBase::ResetToDefaults()
	{
		DefaultsManager::SetNamespace(m_namespace);

		for (int i = 0; i < m_numMembers; i++)
		{
			char const * name = m_names[i];
			SerializableType type = m_types[i];

			switch (type)
			{
				case eSerializableType_float:
					DefaultsManager::GetValue(name, m_floatDefaults[i], m_floatPointers[i]);
					break;
				case eSerializableType_int:
					DefaultsManager::GetValue(name, m_intDefaults[i], m_intPointers[i]);
					break;
			}
		}
	}

	void SerializableBase::FlushChanges()
	{
		DefaultsManager::SetNamespace(m_namespace);

		for (int i = 0; i < m_numMembers; i++)
		{
			char const * name = m_names[i];
			SerializableType type = m_types[i];

			switch (type)
			{
			case eSerializableType_float:
				m_floatDefaults[i] = *m_floatPointers[i];
				DefaultsManager::SetValue(name, *m_floatPointers[i]);
				break;
			case eSerializableType_int:
				m_intDefaults[i] = *m_intPointers[i];
				DefaultsManager::SetValue(name, *m_intPointers[i]);
				break;
			}
		}

		DefaultsManager::Flush();
	}

}