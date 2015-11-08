#pragma once

#include <memory>

#include <bento/core/SceneObject.h>

namespace bento
{
	struct Component : SceneObject
	{
		virtual ~Component();

		Component(std::string _name = "Component", const std::type_info& _typeInfo = typeid(Component));
	};

	typedef std::shared_ptr<Component>	ComponentPtr;
}