#include "Component.h"

namespace bento
{
	Component::Component(std::string _name, const type_info& _typeInfo)
		:SceneObject(_name, _typeInfo)
	{
	}

	Component::~Component()
	{

	}
}
