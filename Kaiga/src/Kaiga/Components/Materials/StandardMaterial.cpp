#include "StandardMaterial.h"


Kaiga::StandardMaterial::StandardMaterial() :
	m_roughness(0.5f),
	m_reflectivity(0.9f),
	m_emissive(0.0f),
	m_diffuse(1.0f, 1.0f, 1.0f)
{

}

float Kaiga::StandardMaterial::Reflectivity() const { return m_reflectivity; }
void Kaiga::StandardMaterial::Reflectivity(float val) { m_reflectivity = val; }

float Kaiga::StandardMaterial::Roughness() const { return m_roughness; }
void Kaiga::StandardMaterial::Roughness(float val) { m_roughness = val; }

float Kaiga::StandardMaterial::Emissive() const { return m_emissive; }
void Kaiga::StandardMaterial::Emissive(float val) { m_emissive = val; }

glm::vec3 Kaiga::StandardMaterial::Diffuse() const { return m_diffuse; }
void Kaiga::StandardMaterial::Diffuse(glm::vec3 val) { m_diffuse = val; }