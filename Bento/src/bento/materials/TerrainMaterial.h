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
		float waterSpecularPower = 80.0f;
		float waterIndexOfRefraction = 0.33f;

		float waveStrength0 = 1.0f;
		float waveStrength1 = 0.5f;
		float waveStrength2 = 0.25f;
		float waveStrength3 = 0.125f;

		float waveAngle0 = 0.0f;
		float waveAngle1 = 4.0f;
		float waveAngle2 = 1.0f;
		float waveAngle3 = 2.0f;

		int waveSpeed0 = 8;
		int waveSpeed1 = 8;
		int waveSpeed2 = 8;
		int waveSpeed3 = 8;

		float waveScale0 = 1.0f;
		float waveScale1 = 2.0f;
		float waveScale2 = 4.0f;
		float waveScale3 = 8.0f;

		float choppyPower = 0.0f;
		float choppyLimit = 1.0f;
		float waveEffectLimit = 0.025f;
		float waveEffectStrength = 0.013f;

		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float directLightIntensity = 1.0f;
		float ambientLightIntensity = 1.2f;
	};
}