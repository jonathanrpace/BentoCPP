#pragma once

#include <vector>
#include <bento.h>

#define SERIALIZABLE(NAME, VALUE)\
SerializableMember(#NAME, VALUE, &NAME);\

namespace bento
{
	enum SerializableType
	{
		eSerializableType_float,
		eSerializableType_int,
		eSerializableType_vec3,
		eSerializableType_vec2,
	};

	class SerializableBase
	{
	public:
		SerializableBase(char * const _namespace);

	protected:
		void SerializableMember(char const * _name, float _default, float* _ptr);
		void SerializableMember(char const * _name, int _default, int* _ptr);
		void SerializableMember(char const * _name, vec3 _default, vec3* _ptr);
		void SerializableMember(char const * _name, vec2 _default, vec2* _ptr);
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

		std::vector<vec3> m_vec3Defaults;
		std::vector<vec3*> m_vec3Pointers;

		std::vector<vec2> m_vec2Defaults;
		std::vector<vec2*> m_vec2Pointers;
	};
}