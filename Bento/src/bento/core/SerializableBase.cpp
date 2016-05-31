#include "SerializableBase.h"

#define VECTOR_SIZE 64

#include <bento/core/DefaultsManager.h>

namespace bento
{
	SerializableBase::SerializableBase(char * const _namespace)
		: m_namespace(_namespace)
		, m_names(VECTOR_SIZE)
		, m_types(VECTOR_SIZE)
		, m_numMembers(VECTOR_SIZE)
		, m_floatDefaults(VECTOR_SIZE)
		, m_floatPointers(VECTOR_SIZE)
		, m_intDefaults(VECTOR_SIZE)
		, m_intPointers(VECTOR_SIZE)
		, m_vec3Defaults(VECTOR_SIZE)
		, m_vec3Pointers(VECTOR_SIZE)
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

	void SerializableBase::SerializableMember(char const * _name, vec3 _default, vec3* _ptr)
	{
		m_names[m_numMembers] = _name;
		m_types[m_numMembers] = eSerializableType_int;
		m_vec3Defaults[m_numMembers] = _default;
		m_vec3Pointers[m_numMembers] = _ptr;
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
				case eSerializableType_vec3:
					DefaultsManager::GetValue(name, m_vec3Defaults[i], m_vec3Pointers[i]);
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
			case eSerializableType_vec3:
				m_vec3Defaults[i] = *m_vec3Pointers[i];
				DefaultsManager::SetValue(name, *m_vec3Pointers[i]);
				break;
			}
		}

		DefaultsManager::Flush();
	}

}