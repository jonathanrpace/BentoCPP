#include "StandardMaterial.h"

namespace bento
{
	StandardMaterial::StandardMaterial(std::string _name)
		: Component(_name)
		, m_roughness(0.5f)
		, m_reflectivity(0.9f)
		, m_emissive(0.0f)
		, m_diffuse(1.0f, 1.0f, 1.0f)
	{

	}

	const std::type_info & StandardMaterial::TypeInfo()
	{
		return typeid(bento::StandardMaterial);
	}

	float StandardMaterial::Reflectivity() const { return m_reflectivity; }
	void StandardMaterial::Reflectivity(float val) { m_reflectivity = val; }

	float StandardMaterial::Roughness() const { return m_roughness; }
	void StandardMaterial::Roughness(float val) { m_roughness = val; }

	float StandardMaterial::Emissive() const { return m_emissive; }
	void StandardMaterial::Emissive(float val) { m_emissive = val; }

	vec3 StandardMaterial::Diffuse() const { return m_diffuse; }
	void StandardMaterial::Diffuse(vec3 val) { m_diffuse = val; }
}

