#pragma once

// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/core/Reflection.h>

namespace bento
{
	class StandardMaterial 
		: public Component
		, public SharedObject<StandardMaterial>
		, public Reflectable
	{
	public:
		StandardMaterial(std::string _name = "StandardMaterial");

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

		DEFINE_REFLECTION_INFO_1(StandardMaterial, float, m_reflectivity);
	};
}