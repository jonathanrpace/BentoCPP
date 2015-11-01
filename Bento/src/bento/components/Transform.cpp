#include "Transform.h"

namespace bento
{
	Transform::Transform(std::string _name)
		: Component(_name)
	{

	}

	const std::type_info & Transform::TypeInfo()
	{
		return typeid(Transform);
	}
}
