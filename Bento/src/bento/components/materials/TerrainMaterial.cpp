#include "TerrainMaterial.h"

#include <imgui.h>

#include <bento/core/DefaultsManager.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, InspectableBase("TerrainMaterial")
		, someTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
		, waterColor(0.219f, 0.286f, 0.278f)
		, waterTranslucentColor(0.219f, 0.411f, 0.392f)
		, lightAltitude((float)-M_PI * 0.5f)
		, lightAzimuth(0.0f)
	{
		someTexture.TexImage2D("textures/DataMap.png");

		// Have seperate label/serialisedKey values?
		// Or same key, but localised? Urgh

		// Rock
		RegisterInspectableMemberSlider("MapHeightOffset", 0.005f, 0.0f, 0.02f, &mapHeightOffset);

		// Water
		RegisterInspectableMemberSlider("SpecularPower", 80.0f, 1.0f, 200.0f, &waterSpecularPower);
		RegisterInspectableMemberSlider("Index Of Refraction", 0.33f, 0.0f, 2.0f, &waterIndexOfRefraction);
		//ImGui::ColorEdit3("WaterColor", glm::value_ptr(waterColor));
		//ImGui::ColorEdit3("WaterTranslucentColor", glm::value_ptr(waterTranslucentColor));

		// Waves
		RegisterInspectableMemberSlider("Effect strength", 
		ImGui::SliderFloat("Effect strength", &waveEffectStrength, 0.0f, 0.05f);
		DefaultsManager::GetValue("waveEffectStrength", 0.013f, &waveEffectStrength);

		ImGui::SliderFloat("Effect limit", &waveEffectLimit, 0.0f, 0.01f, "%.4f");
		DefaultsManager::GetValue("waveEffectLimit", 0.001f, &waveEffectLimit);

		ImGui::SliderFloat("Choppy limit", &choppyLimit, 0.0f, 0.5f);
		DefaultsManager::GetValue("choppyLimit", 1.0f, &choppyLimit);

		ImGui::SliderFloat("Choppy power", &choppyPower, 0.0f, 8.0f);
		DefaultsManager::GetValue("choppyPower", 4.0f, &choppyPower);

		// Wave strength
		ImGui::SliderFloat("Strength 0", &waveStrength0, 0.0f, 2.0f);
		DefaultsManager::GetValue("waveStrength0", 1.0f, &waveStrength0);
		ImGui::SliderFloat("Strength 1", &waveStrength1, 0.0f, 2.0f);
		DefaultsManager::GetValue("waveStrength1", 1.9f, &waveStrength1);
		ImGui::SliderFloat("Strength 2", &waveStrength2, 0.0f, 2.0f);
		DefaultsManager::GetValue("waveStrength2", 0.22f, &waveStrength2);
		ImGui::SliderFloat("Strength 3", &waveStrength3, 0.0f, 2.0f);
		DefaultsManager::GetValue("waveStrength3", 1.0f, &waveStrength3);
		// Wave angle
		ImGui::SliderAngle("Angle 0", &waveAngle0, 0.0f, 360.0f);
		DefaultsManager::GetValue("waveAngle0", 0.0f, &waveStrength3);
		ImGui::SliderAngle("Angle 1", &waveAngle1, 0.0f, 360.0f);
		DefaultsManager::GetValue("waveAngle1", 4.0f, &waveAngle1);
		ImGui::SliderAngle("Angle 2", &waveAngle2, 0.0f, 360.0f);
		DefaultsManager::GetValue("waveAngle2", 1.0f, &waveAngle2);
		ImGui::SliderAngle("Angle 3", &waveAngle3, 0.0f, 360.0f);
		DefaultsManager::GetValue("waveAngle3", 2.0f, &waveAngle3);
		// Wave speed
		ImGui::SliderInt("Speed 0", &waveSpeed0, 0, 64);
		DefaultsManager::GetValue("waveSpeed0", 8, &waveSpeed0);
		ImGui::SliderInt("Speed 1", &waveSpeed1, 0, 64);
		DefaultsManager::GetValue("waveSpeed1", 8, &waveSpeed1);
		ImGui::SliderInt("Speed 2", &waveSpeed2, 0, 64);
		DefaultsManager::GetValue("waveSpeed2", 8, &waveSpeed2);
		ImGui::SliderInt("Speed 3", &waveSpeed3, 0, 64);
		DefaultsManager::GetValue("waveSpeed3", 8, &waveSpeed3);
		// Wave scale
		ImGui::SliderFloat("Scale 0", &waveScale0, 0.0f, 16.0f);
		DefaultsManager::GetValue("waveScale0", 1.0f, &waveScale0);
		ImGui::SliderFloat("Scale 1", &waveScale1, 0.0f, 16.0f);
		DefaultsManager::GetValue("waveScale1", 2.0f, &waveScale1);
		ImGui::SliderFloat("Scale 2", &waveScale2, 0.0f, 16.0f);
		DefaultsManager::GetValue("waveScale2", 4.0f, &waveScale2);
		ImGui::SliderFloat("Scale 3", &waveScale3, 0.0f, 16.0f);
		DefaultsManager::GetValue("waveScale3", 8.0f, &waveScale3);
		
		// Lighting
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 0.0f);
		ImGui::SliderFloat("DirectIntensity", &directLightIntensity, 0.0f, 2.0f);
		DefaultsManager::GetValue("directLightIntensity", 1.0f, &directLightIntensity);
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 2.0f);
		DefaultsManager::GetValue("ambientLightIntensity", 1.2f, &ambientLightIntensity);

		ResetInspectableMembersToDefaults();
	}

}
