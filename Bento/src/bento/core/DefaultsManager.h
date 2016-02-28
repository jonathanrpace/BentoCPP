#pragma once

#include <bento.h>
#include <bento/core/json.hpp>

#include <string.h>

namespace bento
{
	class DefaultsManager
	{
		static void Init(std::string _filepath);

		static void SetNamespace(std::string _namespace);

		template <typename T>
		static T GetValue(std::string _key, T _default);

		template <typename T>
		static void SetValue(std::string _key, T _value);
	};

}
