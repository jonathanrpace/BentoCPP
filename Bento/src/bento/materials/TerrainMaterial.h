#pragma once

// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/textures/TextureSquare.h>

namespace bento
{
	class TerrainMaterial
		: public Component
		, public SharedObject<TerrainMaterial>
	{
	public:
		TerrainMaterial(std::string _name = "TerrainMaterial");

		TextureSquare SomeTexture;
		float MapHeightOffset = 0.005f;
	};
}