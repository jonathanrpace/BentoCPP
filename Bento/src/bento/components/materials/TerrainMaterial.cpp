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

		SerializableMember("waveSpeed", 1.0f, &waveSpeed);
		SerializableMember("waveFrequency", 1.0f, &waveFrequency);
		SerializableMember("waveFrequencyLacunarity", 1.9f, &waveFrequencyLacunarity);
		SerializableMember("waveAmplitude", 0.05f, &waveAmplitude);
		SerializableMember("waveAmplitudeLacunarity", 0.5f, &waveAmplitudeLacunarity);
		SerializableMember("waveChoppy", 4.0f, &waveChoppy);
		SerializableMember("waveChoppyEase", 0.2f, &waveChoppyEase);
		SerializableMember("waveOctavesNum", 8, &waveOctavesNum);
		SerializableMember("waveDepthMap", 0.5, &waveDepthMax);

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
		//ImGui::SliderFloat("Effect strength", &waveEffectStrength, 0.0f, 0.05f);
		//ImGui::SliderFloat("Effect limit", &waveEffectLimit, 0.0f, 0.01f, "%.4f");
		//ImGui::SliderFloat("Choppy limit", &choppyLimit, 0.0f, 0.5f);
		//ImGui::SliderFloat("Choppy power", &choppyPower, 0.0f, 8.0f);
		//ImGui::Text("Wave Strength");
		ImGui::SliderInt("Num octaves", &waveOctavesNum, 1, 16);
		ImGui::SliderFloat("Speed", &waveSpeed, 0.0f, 0.1f);
		ImGui::SliderFloat("Frequency", &waveFrequency, 0.0f, 8.0f);
		ImGui::SliderFloat("Frq Lac", &waveFrequencyLacunarity, 0.0f, 8.0f);
		ImGui::SliderFloat("Amplitude", &waveAmplitude, 0.0f, 0.1f);
		ImGui::SliderFloat("Amp Lac", &waveAmplitudeLacunarity, 0.0f, 2.0f);
		ImGui::SliderFloat("Choppy", &waveChoppy, 0.0f, 8.0f);
		ImGui::SliderFloat("Choppy Ease", &waveChoppyEase, 0.0f, 1.0f);
		ImGui::SliderFloat("Max depth prop", &waveDepthMax, 0.0f, 0.1f);
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
