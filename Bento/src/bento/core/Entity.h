#pragma once

#include <typeinfo>

#include <bento/core/SceneObject.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	struct Entity 
		: SceneObject
		, SharedObject<Entity>
	{
		Entity(std::string _name = "Entity");
	};

	typedef std::shared_ptr<Entity> EntityPtr;
}