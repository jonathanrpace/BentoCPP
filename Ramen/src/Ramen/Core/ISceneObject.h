#pragma once

#include <memory>
#include <typeinfo>

namespace Ramen
{
	class ISceneObject
	{
		public :
			virtual const std::type_info& typeInfo() = 0;
	};
}
