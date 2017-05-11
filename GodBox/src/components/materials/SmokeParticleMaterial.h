#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureResource.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

using namespace bento;

namespace godBox
{
	class SmokeParticleMaterial
		: public Component
		, public SharedObject<SmokeParticleMaterial>
		, public SerializableBase
		, public IInspectable
	{
	public:
		SmokeParticleMaterial(std::string _name = "SmokeParticleMaterial");

		virtual void AddUIElements() override;

		// Textures
		TextureResource texture;

		// Water
		vec3 baseColor;
		vec3 filterColor;

		float density;
		float filterStrength;
	};
}