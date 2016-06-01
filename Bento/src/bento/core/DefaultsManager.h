#pragma once

#include <bento.h>
#include <string.h>

#include <bento/core/json.hpp>

using json = nlohmann::json;

namespace bento
{
	class DefaultsManager
	{

	////////////////////////////////////////////////////////////////////
	// STATIC
	////////////////////////////////////////////////////////////////////
	public:
		static void Init(std::string _filepath);
		static void Shutdown();

		template <typename T>
		static void GetValue(const char* _key, T _default, T* o_value)
		{
			return s_impl->_GetValue(_key, _default, o_value);
		}

		template <typename T>
		static void SetValue(const char* _key, T _value)
		{
			return s_impl->_SetValue(_key, _value);
		}

		static void SetNamespace(std::string _namespace);
		static void Flush();

	private:
		static DefaultsManager* s_impl;

	////////////////////////////////////////////////////////////////////
	// Private implementation
	////////////////////////////////////////////////////////////////////
	public:
		DefaultsManager(std::string _filepath);
		~DefaultsManager();

	private:
		void _Flush();
		void _SetNamespace(std::string _namespace);

		template <typename T>
		void _GetValue(const char* _key, T _default, T* const o_value)
		{
			bool isNull = m_namespace[_key].is_null();
			if (isNull)
			{
				m_namespace[_key] = _default;
				(*o_value) = _default;
				return;
			}
			T value = m_namespace[_key].get<T>();
			(*o_value) = value;
		}

		void _GetValue(const char* _key, vec3 _default, vec3* const o_value);

		template <typename T>
		void _SetValue(const char* _key, T _value)
		{
			m_namespace[_key] = _value;
		}

	
		void _SetValue( const char* _key, vec3 _value);

		std::string m_namespaceName;
		std::string m_filepath;
		json m_data;
		json m_namespace;
	};

}
