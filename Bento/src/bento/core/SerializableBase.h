#pragma once

#include <vector>

namespace bento
{
	enum SerializableType
	{
		eSerializableType_float,
		eSerializableType_int
	};

	class SerializableBase
	{
	public:
		SerializableBase(char * const _namespace);

	protected:
		void SerializableMember(char const * _name, float _default, float* _ptr);
		void SerializableMember(char const * _name, int _default, int* _ptr);
		void ResetToDefaults();
		void FlushChanges();

	private:
		char * const m_namespace;
		std::vector<char const *> m_names;
		std::vector<SerializableType> m_types;
		int m_numMembers;

		std::vector<float> m_floatDefaults;
		std::vector<float*> m_floatPointers;

		std::vector<int> m_intDefaults;
		std::vector<int*> m_intPointers;
	};
}