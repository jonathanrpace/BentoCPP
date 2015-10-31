#include "StandardMaterial.h"


bento::StandardMaterial::StandardMaterial() :
	m_roughness(0.5f),
	m_reflectivity(0.9f),
	m_emissive(0.0f),
	m_diffuse(1.0f, 1.0f, 1.0f)
{

}

const std::type_info & bento::StandardMaterial::typeInfo()
{
	return typeid(bento::StandardMaterial);
}

float bento::StandardMaterial::Reflectivity() const { return m_reflectivity; }
void bento::StandardMaterial::Reflectivity(float val) { m_reflectivity = val; }

float bento::StandardMaterial::Roughness() const { return m_roughness; }
void bento::StandardMaterial::Roughness(float val) { m_roughness = val; }

float bento::StandardMaterial::Emissive() const { return m_emissive; }
void bento::StandardMaterial::Emissive(float val) { m_emissive = val; }

glm::vec3 bento::StandardMaterial::Diffuse() const { return m_diffuse; }
void bento::StandardMaterial::Diffuse(glm::vec3 val) { m_diffuse = val; }

