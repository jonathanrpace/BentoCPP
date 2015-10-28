#pragma once

// std
#include <typeinfo>

// 3rd party
#include <glm.h>

// bento
#include <ramen.h>

namespace Kaiga
{
	class TerrainMaterial
		: public Ramen::IComponent
		, public Ramen::SharedObject<TerrainMaterial>
	{
	public:
		TerrainMaterial();

		// IComponent
		virtual const std::type_info & typeInfo() override;
	};
}