#include "TerrainMaterial.h"

namespace bento
{
	TerrainMaterial::TerrainMaterial()
	{

	}

	const std::type_info & TerrainMaterial::typeInfo()
	{
		return typeid(TerrainMaterial);
	}
}

