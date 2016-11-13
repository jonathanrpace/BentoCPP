#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/components/textures/Texture3D.h>
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
		TextureSquare someTexture;
		TextureSquare moltenPlateDetailTexture;
		TextureSquare smokeTexture;
		TextureSquare rockDiffuseTexture;
		TextureSquare foamTexture;
		TextureSquare creaseTexture;

		// Rock
		vec3 rockColorA;
		vec3 rockColorB;
		float rockRoughnessA;
		float rockRoughnessB;
		float rockFresnelA;
		float rockFresnelB;
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
		float creaseFlowSpeed;
		float creaseFlowOffset;

		// Hot rock
		vec3 hotRockColor;
		float hotRockRoughness;
		float hotRockFresnel;

		// Molten
		vec3 moltenColor;
		float moltenAlphaScalar;
		float moltenAlphaPower;
		float moltenColorScalar;
		float moltenMapOffset; 
		float moltenPlateAlpha;
		float moltenPlateAlphaPower;
		float moltenCreaseAlpha;
		float moltenCreaseAlphaPower;

		// Glow
		float glowScalar;
		float glowPower;
		float glowDetailScalar;
		float glowDetailPower;

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