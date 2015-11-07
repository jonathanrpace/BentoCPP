#include "TerrainMaterial.h"

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name)
		, SomeTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
	{
		SomeTexture.TexImage2D("textures/checker.png");
	}

	const std::type_info & TerrainMaterial::TypeInfo()
	{
		return typeid(TerrainMaterial);
	}
}

