#include "TerrainMaterial.h"

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name)
	{

	}

	const std::type_info & TerrainMaterial::TypeInfo()
	{
		return typeid(TerrainMaterial);
	}
}

