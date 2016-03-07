#pragma once

#include <bento.h>
#include <string.h>

#include <bento/core/json.hpp>

using json = nlohmann::json;

namespace bento
{
	class DefaultsManager
	{
	public:
		static void Init(std::string _filepath);
		static void Flush();
		static void Shutdown();

		static void SetNamespace(std::string _namespace);

		template <typename T>
		static void GetValue(const char* _key, T _default, T* const o_value)
		{
			json::object ns = (*s_data)[(*s_namespace).c_str()].get<json::object>();
			bool isNull = ns[_key].is_null();
			if (isNull)
			{
				(*o_value) = _default;
				return;
			}
			T value = ns[_key].get<T>();
			(*o_value) = value;
		}

		template <typename T>
		static void SetValue(const char* _key, T _value)
		{
			(*s_data)[(*s_namespace).c_str()][_key] = _value;
		}

	private:
		static std::string* s_namespace;
		static std::string* s_filepath;
		static json* s_data;
	};

}
