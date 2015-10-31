#pragma once

// std
#include <typeinfo>

// 3rd party
#include <glm.h>

// bento
#include <bento/core/IComponent.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	class TerrainMaterial
		: public bento::IComponent
		, public bento::SharedObject<TerrainMaterial>
	{
	public:
		TerrainMaterial();

		// IComponent
		virtual const std::type_info & typeInfo() override;
	};
}