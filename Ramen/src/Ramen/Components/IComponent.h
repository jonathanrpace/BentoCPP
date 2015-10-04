#pragma once

#include <typeinfo>

namespace Ramen
{
	class IComponent
	{
	public:
		virtual const std::type_info& typeInfo() = 0;
	};
}