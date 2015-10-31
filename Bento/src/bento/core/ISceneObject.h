#pragma once

#include <memory>
#include <typeinfo>

namespace bento
{
	class ISceneObject
	{
		public :
			virtual const std::type_info& typeInfo() = 0;
	};
}
