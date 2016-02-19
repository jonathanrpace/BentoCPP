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

		float waveStrength0 = 1.0f;
		float waveStrength1 = 1.0f;
		float waveStrength2 = 1.0f;
		float waveStrength3 = 1.0f;

		float waveAngle0 = 0.0f;
		float waveAngle1 = 0.0f;
		float waveAngle2 = 0.0f;
		float waveAngle3 = 0.0f;

		int waveSpeed0 = 1;
		int waveSpeed1 = 1;
		int waveSpeed2 = 1;
		int waveSpeed3 = 1;

		float waveScale0 = 1.0f;
		float waveScale1 = 1.0f;
		float waveScale2 = 1.0f;
		float waveScale3 = 1.0f;

		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float directLightIntensity = 1.0f;
		float ambientLightIntensity = 1.2f;
	};
}