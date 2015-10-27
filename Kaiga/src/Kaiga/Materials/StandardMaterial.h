#pragma once

// std
#include <typeinfo>

// 3rd party
#include <glm.h>

// bento
#include <ramen.h>

namespace Kaiga
{
	class StandardMaterial 
		: public Ramen::IComponent
		, public Ramen::SharedObject<StandardMaterial>
	{
	public:
		StandardMaterial();

		// IComponent
		virtual const std::type_info & typeInfo() override;

		float Reflectivity() const;
		void Reflectivity(float val);

		float Roughness() const;
		void Roughness(float val);

		float Emissive() const;
		void Emissive(float val);

		vec3 Diffuse() const;
		void Diffuse(vec3 val);

	private:
		float m_reflectivity;
		float m_roughness;
		float m_emissive;
		vec3 m_diffuse;
	};
}