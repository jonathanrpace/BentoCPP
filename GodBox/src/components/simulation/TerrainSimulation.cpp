#include "TerrainSimulation.h"

#include <imgui.h>

godBox::TerrainSimulation::TerrainSimulation(std::string _name)
	: Component(_name, typeid(TerrainSimulation))
	, SerializableBase("TerrainSimulation")
{
	// Input
	SERIALIZABLE( mouseRadius,				0.1f );
	SERIALIZABLE( mouseVolumeStrength,		0.002f );
	SERIALIZABLE( mouseHeatStrength,		0.08f );

	// Molten
	SERIALIZABLE( moltenViscosity,			vec2(0.5,0.5) );
	SERIALIZABLE( moltenDamping,			vec2(1.0,1.0) );
	SERIALIZABLE( minMoltenHeat,			0.2f );
	SERIALIZABLE( moltenSlopeStrength,		0.3f );
	SERIALIZABLE( moltenDiffusionStrength,	1.0f );
	SERIALIZABLE( moltenPressureStrength,	1.0f );
	SERIALIZABLE( meltSpeed,				0.01f );
	SERIALIZABLE( condenseSpeed,			0.01f );
	SERIALIZABLE( tempChangeSpeed,			0.002f );
	SERIALIZABLE( smudgeChangeRate,			0.01f );

	// Water										 
	SERIALIZABLE( waterViscosity,			0.25f );
	SERIALIZABLE( waterSlopeStrength,		0.3f );
	SERIALIZABLE( waterBoilingPoint,		0.1f );
	SERIALIZABLE( waterFreezingPoint,		0.0f );
	SERIALIZABLE( evapourationRate,			0.0f );
	SERIALIZABLE( rainRate,					0.0f );
	SERIALIZABLE( boilSpeed,				0.001f );
	SERIALIZABLE( drainRate,				0.0f );
	SERIALIZABLE( drainMaxDepth,			0.01f );

	// Dirt	
	SERIALIZABLE( dirtViscosity,			0.2f );
	SERIALIZABLE( dirtMaxSlope,				0.2f );
	SERIALIZABLE( dirtDensity,				0.5f );

	// Erosion										 
	SERIALIZABLE( erosionStrength,			0.0f );
	SERIALIZABLE( erosionDirtDepthMax,		0.01f );
	SERIALIZABLE( erosionWaterDepthMin,		0.01f );
	SERIALIZABLE( erosionWaterDepthMax,		0.01f );
	SERIALIZABLE( erosionWaterSpeedMax,		0.01f );

	// Dirt transport								 
	SERIALIZABLE( dirtTransportSpeed,		0.0f );
	SERIALIZABLE( dirtPickupRate,			0.0f );
	SERIALIZABLE( dirtPickupMaxWaterSpeed,	1.0f );
	SERIALIZABLE( dirtDepositSpeed,			0.0f );

	// Global										 
	SERIALIZABLE( ambientTemperature,		0.05f );

	ResetToDefaults();
}

void godBox::TerrainSimulation::AddUIElements()
{
	ImGui::Spacing();
	ImGui::Text("Input");
	ImGui::SliderFloat("MouseRadius", &mouseRadius, 0.01f, 0.5f);
	ImGui::SliderFloat("MouseVolumeStrength", &mouseVolumeStrength, 0.00f, 0.01f, "%.5f");
	ImGui::SliderFloat("MouseHeatStrength", &mouseHeatStrength, 0.00f, 0.5f, "%.2f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Environment");
	ImGui::SliderFloat("AmbientTemp", &ambientTemperature, -1.0f, 1.0f);
	ImGui::Spacing();

	ImGui::Text("Molten");
	ImGui::SliderFloat2("Viscosity##molten", &moltenViscosity.x, 0.0f, 1.0f);
	ImGui::SliderFloat2("Damping##molten", &moltenDamping.x, 0.9f, 1.0f);
	ImGui::SliderFloat("Min Heat##molten", &minMoltenHeat, 0.0f, 1.0f);
	ImGui::SliderFloat("Slope Strength##molten", &moltenSlopeStrength, 0.0f, 2.0f, "%.4f");
	ImGui::SliderFloat("Diffusion##molten", &moltenDiffusionStrength, 0.0f, 1.0f, "%.4f");
	ImGui::SliderFloat("Pressure Strength##molten", &moltenPressureStrength, 0.0f, 1.0f, "%.3f");
	ImGui::SliderFloat("TempChangeSpeed", &tempChangeSpeed, 0.0f, 0.01f, "%.5f");
	ImGui::DragFloat("Melt Speed", &meltSpeed, 0.0001f, 0.0f, 0.0f, "%.5f");
	ImGui::DragFloat("Condense Speed", &condenseSpeed, 0.0001f, 0.0f, 0.0f, "%.5f");
	ImGui::SliderFloat("SmudgeChangeRate", &smudgeChangeRate, 0.0f, 10.0f, "%.5f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Water");
	ImGui::SliderFloat("Viscosity##water", &waterViscosity, 0.2f, 1.0f);
	ImGui::SliderFloat("Slope Strength##water", &waterSlopeStrength, 0.0f, 2.0f, "%.4f");
	ImGui::SliderFloat("RainRate", &rainRate, 0.00f, 0.00001f, "%.8f");
	ImGui::SliderFloat("EvapourationRate", &evapourationRate, 0.00f, 0.0001f, "%.8f");
	ImGui::SliderFloat("DrainRate", &drainRate, 0.0f, 0.001f, "%.5f");
	ImGui::SliderFloat("DrainMaxDepth", &drainMaxDepth, 0.0f, 0.05f, "%.3f");
	ImGui::SliderFloat("BoilSpeed", &boilSpeed, 0.0f, 0.001f, "%.4f");
	ImGui::Spacing();

	ImGui::Spacing();
	ImGui::Text("Dirt");
	ImGui::SliderFloat("Viscosity##dirt", &dirtViscosity, 0.01f, 1.0f);
	ImGui::SliderFloat("MaxSlope##dirt", &dirtMaxSlope, 0.0f, 0.01f, "%.5f");
	ImGui::SliderFloat("Density##dirt", &dirtDensity, 0.0f, 1.0f, "%.4f");
	ImGui::Spacing();

	ImGui::Text("Erosion");
	ImGui::SliderFloat("Strength", &erosionStrength, 0.0f, 0.0001f, "%.9f");
	ImGui::SliderFloat("DirtDepthMax", &erosionDirtDepthMax, 0.0f, 0.03f, "%.4f");
	ImGui::SliderFloat("WaterSpeedMax##erode", &erosionWaterSpeedMax, 0.0f, 0.1f, "%.3f");

	ImGui::Text("Dirt Transport");
	ImGui::SliderFloat("WaterSpeedMax##pickup", &dirtPickupMaxWaterSpeed, 0.0f, 0.1f, "%.7f");
	ImGui::SliderFloat("PickupRate", &dirtPickupRate, 0.0f, 0.0001f, "%.7f");
	ImGui::SliderFloat("DepositRate", &dirtDepositSpeed, 0.0f, 0.05f, "%.7f");
	ImGui::Spacing();

	if (ImGui::Button("Reset"))
	{
		ResetToDefaults();
	}
	if (ImGui::Button("Save"))
	{
		FlushChanges();
	}
}
