#pragma once

#define _USE_MATH_DEFINES
#include <math.h>


// bento
#include <bento.h>
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/components/textures/TextureResource.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

using namespace bento;

namespace godBox
{
	class CloudMaterial
		: public Component
		, public SharedObject<CloudMaterial>
		, public SerializableBase
		, public IInspectable
	{
	public:
		CloudMaterial(std::string _name = "CloudMaterial");

		virtual void AddUIElements() override;

		// Textures
		TextureResource baseShapesTexture3D;
		TextureResource detailTexture3D;

		float baseScale;
		float baseScaleVertical;
		float detailScale;
		float detailScaleVertical;
		float detailMaxDensity;
		
		

		float position;
		float height;
		float falloffBottom;
		float falloffTop;

		float maxRayLength;
		int coarseStepsPerUnit;
		int detailStepsPerUnit;
		float lightConeMaxLength;

		float lightAzimuth;
		float lightAltitude;
		vec3 lightColor;
		vec3 skyColor;
		float lightIntensity;
		float absorbtion;
		float powderSharpness;
		float baseDensityScalar;
		float baseDensityOffset;
		float scatteringParam;

		float detailDensityScalar;
		float detailDensityOffset;
	};
}