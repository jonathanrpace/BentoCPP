#pragma once

// std
#include <typeinfo>

// 3rd party
#include <glm.h>

// ramen
#include <Ramen/Core/IComponent.h>
#include <Ramen/Core/SharedObject.h>
#include <Ramen/Reflection.h>

namespace Kaiga
{
	class StandardMaterial 
		: public Ramen::IComponent
		, public Ramen::SharedObject<StandardMaterial>
		, public Ramen::Reflectable
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

		DEFINE_REFLECTION_INFO_1(StandardMaterial, float, m_reflectivity);
	};
}