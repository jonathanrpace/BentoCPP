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
	class WaterMaterial
		: public Component
		, public SharedObject<WaterMaterial>
		, public SerializableBase
		, public IInspectable
	{
	public:
		WaterMaterial(std::string _name = "WaterMaterial");

		virtual void AddUIElements() override;

		// Textures
		TextureResource foamTexture;

		// Water
		vec3 filterColor;
		vec3 scatterColor;

		float reflectivity;
		float indexOfRefraction;
		float depthToReflection;
		float depthToFilter;
		float depthToDiffuse;
		float depthPower;
		float localDepthMip;
		float localDepthScalar;
		float localDepthValue;
		float dirtScalar;
		float flowSpeed;
		float flowOffset;

		// Waves
		int   waveLevels;
		float waveAmplitude;
		float waveFrquencyBase;
		float waveFrquencyScalar;
		float waveRoughness;
		float waveChoppy;
		float waveChoppyEase;
		float waveSpeed;
		
		// Foam
		float foamRepeat;
		float foamStrength;
	};
}