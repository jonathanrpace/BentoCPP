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
		TextureSquare steamTexture;
		TextureSquare smokeTexture;
		TextureSquare rockDiffuseTexture;
		Texture3D texture3D;

		// Rock
		vec3 rockColorA;
		vec3 rockColorB;
		float rockRoughnessA;
		float rockRoughnessB;
		float rockFresnelA;
		float rockFresnelB;
		float rockDetailDiffuseStrength;
		float rockDetailBumpStrength;

		// Hot rock
		vec3 hotRockColorA;
		vec3 hotRockColorB;
		float hotRockRoughnessA;
		float hotRockRoughnessB;
		float hotRockFresnelA;
		float hotRockFresnelB;

		// Molten
		vec3 moltenColor;
		float moltenMapAlphaScalar;
		float moltenMapAlphaPower;
		float moltenMapOffset; 

		// Dirt
		vec3 dirtColor;
		float dirtTextureRepeat;

		// Water
		vec3 waterColor;
		float waterSpecularPower;
		float waterIndexOfRefraction;
		float waterDepthToOpaque;
		float waterDepthToDiffuse;
		float dissolvedDirtDepthToDiffuse;

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