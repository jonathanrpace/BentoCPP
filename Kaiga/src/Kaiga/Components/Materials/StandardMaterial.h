#pragma once

#include <glm/glm.hpp>

#include <Ramen\Components\ComponentBase.h>

namespace Kaiga
{
	class StandardMaterial :
		public Ramen::ComponentBase<StandardMaterial>
	{
	public:
		StandardMaterial();

		float Reflectivity() const;
		void Reflectivity(float val);

		float Roughness() const;
		void Roughness(float val);

		float Emissive() const;
		void Emissive(float val);

		glm::vec3 Diffuse() const;
		void Diffuse(glm::vec3 val);

	private:
		float m_reflectivity;
		float m_roughness;
		float m_emissive;
		glm::vec3 m_diffuse;
	};
}