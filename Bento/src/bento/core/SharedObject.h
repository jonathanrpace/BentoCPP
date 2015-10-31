#pragma once

#include <memory>

namespace bento
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