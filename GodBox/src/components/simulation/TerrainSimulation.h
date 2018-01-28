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
	class TerrainSimulation
		: public Component
		, public SharedObject<TerrainSimulation>
		, public SerializableBase
		, public IInspectable
	{
	public:
		TerrainSimulation(std::string _name = "TerrainSimulation");

		virtual void AddUIElements() override;

		// Input
		float mouseRadius;
		float mouseVolumeStrength;
		float mouseHeatStrength;

		// Molten
		vec2 moltenViscosity;
		vec2 moltenDamping;
		float minMoltenHeat;
		float moltenSlopeStrength;
		float moltenDiffusionStrength;
		float moltenPressureStrength;
		float meltSpeed;
		float condenseSpeed;
		float tempChangeSpeed;
		float smudgeChangeRate;
		
		// Dirt
		float dirtViscosity;
		float dirtMaxSlope;
		float dirtDensity;

		// Water
		float waterViscosity;
		float waterSlopeStrength;
		float waterBoilingPoint;
		float waterFreezingPoint;
		float evapourationRate;
		float rainRate;
		float boilSpeed;
		float drainRate;
		float drainMaxDepth;

		// Erosion
		float erosionStrength;
		float erosionWaterSpeedMax;
		float erosionWaterDepthMin;
		float erosionWaterDepthMax;
		float erosionDirtDepthMax;

		// Dirt transport
		float dirtTransportSpeed;
		float dirtPickupRate;
		float dirtPickupMinWaterSpeed;
		float dirtDepositSpeed;
		float dissolvedDirtSmoothing;

		// Global
		float ambientTemperature;
	};
}