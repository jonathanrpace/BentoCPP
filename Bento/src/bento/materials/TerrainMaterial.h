#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/textures/TextureSquare.h>
#include <bento/core/IInspectable.h>

namespace bento
{
	class TerrainMaterial
		: public Component
		, public SharedObject<TerrainMaterial>
		, public IInspectable
	{
	public:
		TerrainMaterial(std::string _name = "TerrainMaterial");

		virtual void AddUIElements() override;

		TextureSquare SomeTexture;
		float MapHeightOffset = 0.005f;

		// Water
		vec3 waterColor;
		vec3 waterTranslucentColor;
		float waterSpecularPower = 125.0f;
		float waterIndexOfRefraction = 0.0f;

		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float directLightIntensity = 1.0f;
		float ambientLightIntensity = 1.2f;
	};
}