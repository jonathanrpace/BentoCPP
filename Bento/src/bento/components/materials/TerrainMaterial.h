#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureSquare.h>
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

		void ResetDefaults();
		virtual void AddUIElements() override;

		TextureSquare SomeTexture;
		float MapHeightOffset;

		// Water
		vec3 waterColor;
		vec3 waterTranslucentColor;
		float waterSpecularPower;
		float waterIndexOfRefraction;

		float waveStrength0;
		float waveStrength1;
		float waveStrength2;
		float waveStrength3;

		float waveAngle0;
		float waveAngle1;
		float waveAngle2;
		float waveAngle3;

		int waveSpeed0;
		int waveSpeed1;
		int waveSpeed2;
		int waveSpeed3;

		float waveScale0;
		float waveScale1;
		float waveScale2;
		float waveScale3;

		float choppyPower;
		float choppyLimit;
		float waveEffectLimit;
		float waveEffectStrength;

		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float directLightIntensity;
		float ambientLightIntensity;
	};
}