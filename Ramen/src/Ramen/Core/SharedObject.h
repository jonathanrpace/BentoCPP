#pragma once

#include <memory>

namespace Ramen
{
	template<typename T>
	class SharedObject
	{
	public:
		static std::shared_ptr<T> Create()
		{
			return std::make_shared<T>();
		}
	};
}