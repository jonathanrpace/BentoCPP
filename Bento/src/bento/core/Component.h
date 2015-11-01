#pragma once

#include <bento/core/SceneObject.h>

namespace bento
{
	struct Component : SceneObject
	{
		Component(std::string _name = "Component");
	};

	typedef std::shared_ptr<Component>	ComponentPtr;
}