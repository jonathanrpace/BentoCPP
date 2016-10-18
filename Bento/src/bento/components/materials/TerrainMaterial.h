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

		// Textures
		TextureSquare someTexture;
		TextureSquare moltenPlateDetailTexture;
		TextureSquare smokeTexture;
		TextureSquare rockDiffuseTexture;
		TextureSquare foamTexture;

		// Rock
		vec3 rockColorA;
		vec3 rockColorB;
		float rockRoughnessA;
		float rockRoughnessB;
		float rockFresnelA;
		float rockFresnelB;
		float rockDetailDiffuseStrength;
		float rockDetailBumpStrength;

		// Creases
		float bearingCreaseScalar;
		float lateralCreaseScalar;
		float creaseForwardScalar;
		float creaseMipLevel;

		// Hot rock
		vec3 hotRockColor;
		float hotRockRoughness;
		float hotRockFresnel;

		// Molten
		vec3 moltenColor;
		float moltenMapAlphaScalar;
		float moltenMapAlphaPower;
		float moltenMapOffset; 

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