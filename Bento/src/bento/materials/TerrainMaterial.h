#pragma once

// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>

namespace bento
{
	class TerrainMaterial
		: public Component
		, public SharedObject<TerrainMaterial>
	{
	public:
		TerrainMaterial(std::string _name = "TerrainMaterial");

		// IComponent
		virtual const std::type_info & TypeInfo() override;
	};
}