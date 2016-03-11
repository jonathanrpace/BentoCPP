#include "TerrainMaterial.h"

#include <imgui.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SerializableBase("TerrainMaterial")
		, someTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
		, waterColor(0.219f, 0.286f, 0.278f)
		, waterTranslucentColor(0.219f, 0.411f, 0.392f)
	{
		someTexture.TexImage2D("textures/DataMap.png");

		SerializableMember("mapHeightOffset", 0.005f, &mapHeightOffset);
		SerializableMember("waterSpecularPower", 80.0f, &waterSpecularPower);
		SerializableMember("waterIndexOfRefraction", 0.33f, &waterIndexOfRefraction);
		//SerializableMember("waterColor", 0.33f, &waterIndexOfRefraction);
		//SerializableMember("waterTranslucentColor", 0.33f, &waterTranslucentColor);
		SerializableMember("waveEffectStrength", 0.013f, &waveEffectStrength);
		SerializableMember("waveEffectLimit", 0.001f, &waveEffectLimit);
		SerializableMember("choppyLimit", 1.0f, &choppyLimit);
		SerializableMember("choppyPower", 4.0f, &choppyPower);
		SerializableMember("waveStrength0", 1.0f, &waveStrength0);
		SerializableMember("waveStrength1", 1.9f, &waveStrength1);
		SerializableMember("waveStrength2", 0.22f, &waveStrength2);
		SerializableMember("waveStrength3", 1.0f, &waveStrength3);
		SerializableMember("waveAngle0", 0.0f, &waveStrength3);
		SerializableMember("waveAngle1", 4.0f, &waveAngle1);
		SerializableMember("waveAngle2", 1.0f, &waveAngle2);
		SerializableMember("waveAngle3", 2.0f, &waveAngle3);
		SerializableMember("waveSpeed0", 8, &waveSpeed0);
		SerializableMember("waveSpeed1", 8, &waveSpeed1);
		SerializableMember("waveSpeed2", 8, &waveSpeed2);
		SerializableMember("waveSpeed3", 8, &waveSpeed3);
		SerializableMember("waveScale0", 1.0f, &waveScale0);
		SerializableMember("waveScale1", 2.0f, &waveScale1);
		SerializableMember("waveScale2", 4.0f, &waveScale2);
		SerializableMember("waveScale3", 8.0f, &waveScale3);
		SerializableMember("lightAzimuth", 0.0f, &lightAzimuth);
		SerializableMember("lightAltitude", (float)-M_PI * 0.5f, &lightAltitude);
		SerializableMember("directLightIntensity", 1.0f, &directLightIntensity);
		SerializableMember("ambientLightIntensity", 1.2f, &ambientLightIntensity);

		ResetToDefaults();
	}

	void TerrainMaterial::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Rock");
		ImGui::SliderFloat("BumpStrength", &mapHeightOffset, 0.0f, 0.02f);
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

		if (ImGui::Button("Reset"))
		{
			ResetToDefaults();
		}
		if (ImGui::Button("Save"))
		{
			FlushChanges();
		}
	}

}
