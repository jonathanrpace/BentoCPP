#include "TerrainMaterial.h"

#include <imgui.h>

#include <bento/core/DefaultsManager.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SomeTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
		, waterColor(0.219f, 0.286f, 0.278f)
		, waterTranslucentColor(0.219f, 0.411f, 0.392f)
		, lightAltitude((float)-M_PI * 0.5f)
		, lightAzimuth(0.0f)
	{
		SomeTexture.TexImage2D("textures/DataMap.png");

		ResetDefaults();
	}

	void TerrainMaterial::ResetDefaults()
	{
		DefaultsManager::SetNamespace("TerrainMaterial");
		DefaultsManager::GetValue("MapHeightOffset", 0.005f, &MapHeightOffset);
		DefaultsManager::GetValue("waterSpecularPower", 80.0f, &waterSpecularPower);
		DefaultsManager::GetValue("waterIndexOfRefraction", 0.33f, &waterIndexOfRefraction);
		DefaultsManager::GetValue("waveStrength0", 1.0f, &waveStrength0);
		DefaultsManager::GetValue("waveStrength1", 1.9f, &waveStrength1);
		DefaultsManager::GetValue("waveStrength2", 0.22f, &waveStrength2);
		DefaultsManager::GetValue("waveStrength3", 1.0f, &waveStrength3);
		DefaultsManager::GetValue("waveAngle0", 0.0f, &waveStrength3);
		DefaultsManager::GetValue("waveAngle1", 4.0f, &waveAngle1);
		DefaultsManager::GetValue("waveAngle2", 1.0f, &waveAngle2);
		DefaultsManager::GetValue("waveAngle3", 2.0f, &waveAngle3);
		DefaultsManager::GetValue("waveSpeed0", 8, &waveSpeed0);
		DefaultsManager::GetValue("waveSpeed1", 8, &waveSpeed1);
		DefaultsManager::GetValue("waveSpeed2", 8, &waveSpeed2);
		DefaultsManager::GetValue("waveSpeed3", 8, &waveSpeed3);
		DefaultsManager::GetValue("waveScale0", 1.0f, &waveScale0);
		DefaultsManager::GetValue("waveScale1", 2.0f, &waveScale1);
		DefaultsManager::GetValue("waveScale2", 4.0f, &waveScale2);
		DefaultsManager::GetValue("waveScale3", 8.0f, &waveScale3);
		DefaultsManager::GetValue("choppyPower", 4.0f, &choppyPower);
		DefaultsManager::GetValue("choppyLimit", 1.0f, &choppyLimit);
		DefaultsManager::GetValue("waveEffectLimit", 0.001f, &waveEffectLimit);
		DefaultsManager::GetValue("waveEffectStrength", 0.013f, &waveEffectStrength);
		DefaultsManager::GetValue("directLightIntensity", 1.0f, &directLightIntensity);
		DefaultsManager::GetValue("ambientLightIntensity", 1.2f, &ambientLightIntensity);
	}

	void TerrainMaterial::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Rock");
		ImGui::SliderFloat("BumpStrength", &MapHeightOffset, 0.0f, 0.02f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Water");
		ImGui::SliderFloat("SpecularPower", &waterSpecularPower, 1.0f, 200.0f);
		ImGui::SliderFloat("I.O.R", &waterIndexOfRefraction, 0.0f, 2.0f);
		ImGui::ColorEdit3("WaterColor", glm::value_ptr(waterColor));
		ImGui::ColorEdit3("WaterTranslucentColor", glm::value_ptr(waterTranslucentColor));
		ImGui::Text("Waves");
		ImGui::SliderFloat("Effect strength", &waveEffectStrength, 0.0f, 0.05f);
		ImGui::SliderFloat("Effect limit", &waveEffectLimit, 0.0f, 0.01f, "%.4f");
		ImGui::SliderFloat("Choppy limit", &choppyLimit, 0.0f, 0.5f);
		ImGui::SliderFloat("Choppy power", &choppyPower, 0.0f, 8.0f);
		ImGui::Text("Wave Strength");
		ImGui::SliderFloat("Strength 0", &waveStrength0, 0.0f, 2.0f);
		ImGui::SliderFloat("Strength 1", &waveStrength1, 0.0f, 2.0f);
		ImGui::SliderFloat("Strength 2", &waveStrength2, 0.0f, 2.0f);
		ImGui::SliderFloat("Strength 3", &waveStrength3, 0.0f, 2.0f);
		ImGui::Text("Wave Angle");
		ImGui::SliderAngle("Angle 0", &waveAngle0, 0.0f, 360.0f);
		ImGui::SliderAngle("Angle 1", &waveAngle1, 0.0f, 360.0f);
		ImGui::SliderAngle("Angle 2", &waveAngle2, 0.0f, 360.0f);
		ImGui::SliderAngle("Angle 3", &waveAngle3, 0.0f, 360.0f);
		ImGui::Text("Wave Speed");
		ImGui::SliderInt("Speed 0", &waveSpeed0, 0, 64);
		ImGui::SliderInt("Speed 1", &waveSpeed1, 0, 64);
		ImGui::SliderInt("Speed 2", &waveSpeed2, 0, 64);
		ImGui::SliderInt("Speed 3", &waveSpeed3, 0, 64);
		ImGui::Text("Wave Scale");
		ImGui::SliderFloat("Scale 0", &waveScale0, 0.0f, 16.0f);
		ImGui::SliderFloat("Scale 1", &waveScale1, 0.0f, 16.0f);
		ImGui::SliderFloat("Scale 2", &waveScale2, 0.0f, 16.0f);
		ImGui::SliderFloat("Scale 3", &waveScale3, 0.0f, 16.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Lighting");
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 0.0f);
		ImGui::SliderFloat("DirectIntensity", &directLightIntensity, 0.0f, 2.0f);
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 2.0f);
		ImGui::Spacing();

		if (ImGui::Button("Save"))
		{
			DefaultsManager::Flush();
		}

		if (ImGui::Button("Revert"))
		{
			ResetDefaults();
		}
	}

}
