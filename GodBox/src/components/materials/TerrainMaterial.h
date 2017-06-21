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

		// Textures|
		TextureResource grungeTexture;
		TextureResource smokeTexture;
		TextureResource envMap;
		TextureResource irrMap;

		TextureResource lavaAlb;
		TextureResource lavaNrm;
		TextureResource lavaMat;

		TextureResource moltenGradient;
		TextureResource albedoFluidGradient;

		// Global
		float heightOffset;
		float uvRepeat;

		// Rock
		float rockReflectivity;
		float rockNormalStrength;
		float rockDetailBumpSlopePower;

		// Smudge
		float smudgeUVStrength;

		// Creases
		float creaseSampleOffset;
		float creaseSampleMip;
		float creaseFrequency;
		float creaseNormalStrength;
		float creaseDistortStrength;

		// Molten
		float moltenFlowSpeed;
		float moltenFlowOffset;
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