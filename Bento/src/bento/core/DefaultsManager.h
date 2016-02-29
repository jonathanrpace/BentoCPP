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
		static void Shutdown();

		static void SetNamespace(std::string _namespace);

		template <typename T>
		static T GetValue(const char* _key, T _default)
		{
			return s_data[_key].get<T>();
		}

		template <typename T>
		static void SetValue(const char* _key, T _value)
		{
			s_data[_key] = _value;
		}

	private:
		static std::string s_namespace;
		static json s_data;
	};

}
