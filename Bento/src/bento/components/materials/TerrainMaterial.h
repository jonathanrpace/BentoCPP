#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

namespace bento
{
	class TerrainMaterial
		: public Component
		, public SharedObject<TerrainMaterial>
		, public SerializableBase
		, public IInspectable
	{
	public:
		TerrainMaterial(std::string _name = "TerrainMaterial");

		virtual void AddUIElements() override;

		TextureSquare someTexture;
		float mapHeightOffset;

		// Water
		vec3 waterColor;
		vec3 waterTranslucentColor;
		float waterSpecularPower;
		float waterIndexOfRefraction;

		float waveSpeed;
		float waveFrequency;
		float waveFrequencyLacunarity;
		float waveAmplitude;
		float waveAmplitudeLacunarity;
		float waveChoppy;
		float waveChoppyEase;
		int waveOctavesNum;

		float waveDepthMax;
		//float waveEffectStrength;
		
		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float directLightIntensity;
		float ambientLightIntensity;

		// Fog
		float fogDensity;
		float fogHeight;
		vec3 fogColorAway;
		vec3 fogColorTowards;
	};
}