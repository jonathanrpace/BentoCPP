#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/components/textures/TextureResource.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

using namespace bento;

namespace godBox
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

		// Textures
		TextureResource grungeTexture;
		TextureResource smokeTexture;
		TextureResource foamTexture;
		TextureResource envMap;

		TextureResource lavaAlb;
		TextureResource lavaNrm;
		TextureResource lavaMat;

		TextureResource lavaLongAlb;
		TextureResource lavaLongNrm;
		TextureResource lavaLongMat;

		TextureResource lavaLatAlb;
		TextureResource lavaLatNrm;
		TextureResource lavaLatMat;

		// Global
		float heightOffset;

		// Rock
		float rockReflectivity;
		float rockDetailBumpStrength;
		float rockDetailBumpSlopePower;

		// Creases
		float bearingCreaseScalar;
		float lateralCreaseScalar;
		float creaseForwardScalar;
		float creaseRatio;
		float creaseMipLevel;
		float creaseMapRepeat;
		float creaseGridRepeat;
		float moltenFlowSpeed;
		float moltenFlowOffset;

		// Molten
		vec3 moltenColor;
		float moltenAlphaScalar;
		float moltenAlphaPower;
		float moltenColorScalar;

		// Glow
		float glowScalar;

		// Dirt
		vec3 dirtColor;
		float dirtTextureRepeat;
		float dirtHeightToOpaque;

		// Water
		vec3 waterColor;
		float waterSpecularPower;
		float waterIndexOfRefraction;
		float waterDepthToReflect;
		float waterDepthToFilter;
		float waterDepthToDiffuse;
		float dissolvedDirtDesntiyScalar;
		float waterFlowSpeed;
		float waterFlowOffset;
		float waterFlowRepeat;

		// Waves
		int waterWaveLevels;
		float waterWaveAmplitude;
		float waterWaveFrquencyBase;
		float waterWaveFrquencyScalar;
		float waterWaveRoughness;
		float waterWaveSpeed;
		
		// Foam
		float foamRepeat;
		float foamDistortStrength;
		float foamAlphaStrength;

		// Lighting
		float lightAzimuth;
		float lightAltitude;
		float lightDistance;
		float directLightIntensity;
		float ambientLightIntensity;

		// Fog
		float fogDensity;
		float fogHeight;
		vec3 fogColorAway;
		vec3 fogColorTowards;
	};
}