#include "TerrainMaterial.h"

#include <imgui.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SerializableBase("TerrainMaterial")
		, someTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
		, moltenPlatesTexture(16, GL_RGBA8, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP)
	{
		// Textures
		someTexture.TexImage2D("textures/DataMap.png");
		moltenPlatesTexture.TexImage2D("textures/MoltenPlates.png");

		// Rock
		SerializableMember("rockColorA", vec3(0.1f, 0.1f, 0.1f), &rockColorA);
		SerializableMember("rockColorB", vec3(0.1f, 0.1f, 0.1f), &rockColorB);
		SerializableMember("rockRoughnessA", 0.1f, &rockRoughnessA);
		SerializableMember("rockRoughnessB", 0.0f, &rockRoughnessB);
		SerializableMember("rockFresnelA", 1.0f, &rockFresnelA);
		SerializableMember("rockFresnelB", 1.0f, &rockFresnelB);

		// Hot rock
		SerializableMember("hotRockColorA", vec3(0.1f, 0.1f, 0.1f), &hotRockColorA);
		SerializableMember("hotRockColorB", vec3(0.1f, 0.1f, 0.1f), &hotRockColorB);
		SerializableMember("hotRockRoughnessA", 0.0f, &hotRockRoughnessA);
		SerializableMember("hotRockRoughnessB", 0.1f, &hotRockRoughnessB);
		SerializableMember("hotRockFresnelA", 1.0f, &hotRockFresnelA);
		SerializableMember("hotRockFresnelB", 1.0f, &hotRockFresnelB);

		// Molten
		SerializableMember("moltenColor", vec3(1.0f, 0.5f, 0.01f), &moltenColor);
		SerializableMember("moltenMapAlphaScalar", 1.0f, &moltenMapAlphaScalar);
		SerializableMember("moltenMapAlphaPower", 1.0f, &moltenMapAlphaPower);

		// Water
		SerializableMember("waterColor", vec3(0.219f, 0.286f, 0.278f), &waterColor);
		SerializableMember("waterTranslucentColor", vec3(0.219f, 0.411f, 0.392f), &waterTranslucentColor);
		SerializableMember("waterSpecularPower", 80.0f, &waterSpecularPower);
		SerializableMember("waterIndexOfRefraction", 0.33f, &waterIndexOfRefraction);

		// Lighting
		SerializableMember("lightAzimuth", 0.0f, &lightAzimuth);
		SerializableMember("lightAltitude", (float)-M_PI * 0.5f, &lightAltitude);
		SerializableMember("directLightIntensity", 1.0f, &directLightIntensity);
		SerializableMember("ambientLightIntensity", 1.2f, &ambientLightIntensity);

		// Fog
		SerializableMember("fogColorAway", vec3(0.7f, 0.7f, 0.7f), &fogColorAway);
		SerializableMember("fogColorTowards", vec3(1.0f, 0.9f, 0.7f), &fogColorTowards);
		SerializableMember("fogDensity", 0.2f, &fogDensity);
		SerializableMember("fogHeight", 0.2f, &fogHeight);
		SerializableMember("fogFalloff", 1.0f, &fogFalloff);

		ResetToDefaults();
	}

	void TerrainMaterial::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color A##rock", glm::value_ptr(rockColorA));
		ImGui::ColorEdit3("Color B##rock", glm::value_ptr(rockColorB));
		ImGui::SliderFloat("Roughness A##rock", &rockRoughnessA, 0.0f, 1.0f);
		ImGui::SliderFloat("Roughness B##rock", &rockRoughnessB, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel A##rock", &rockFresnelA, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel B##rock", &rockFresnelB, 0.0f, 1.0f);

		ImGui::Spacing();
		ImGui::Text("Hot rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color B##hotRock", glm::value_ptr(hotRockColorB));
		ImGui::ColorEdit3("Color A##hotRock", glm::value_ptr(hotRockColorA));
		ImGui::SliderFloat("Roughness A##hotRock", &hotRockRoughnessA, 0.0f, 1.0f);
		ImGui::SliderFloat("Roughness B##hotRock", &hotRockRoughnessB, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel A##hotRock", &hotRockFresnelA, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel B##hotRock", &hotRockFresnelB, 0.0f, 1.0f);

		ImGui::Spacing();
		ImGui::Text("Molten");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color##molten", glm::value_ptr(moltenColor));
		ImGui::SliderFloat("Alpha Scale##molten", &moltenMapAlphaScalar, 0.0f, 4.0f);
		ImGui::SliderFloat("Alpha Power##molten", &moltenMapAlphaPower, 0.0f, 4.0f);

		ImGui::Spacing();
		ImGui::Text("Water");
		ImGui::SliderFloat("SpecularPower", &waterSpecularPower, 1.0f, 200.0f);
		ImGui::SliderFloat("I.O.R", &waterIndexOfRefraction, 0.0f, 2.0f);
		ImGui::ColorEdit3("WaterColor", glm::value_ptr(waterColor));
		ImGui::ColorEdit3("WaterTranslucentColor", glm::value_ptr(waterTranslucentColor));
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Lighting");
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 0.0f);
		ImGui::SliderFloat("DirectIntensity", &directLightIntensity, 0.0f, 2.0f);
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 2.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Fog");
		ImGui::SliderFloat("Density", &fogDensity, 0.0f, 100.0f);
		ImGui::SliderFloat("Height", &fogHeight, 0.0f, 0.5f);
		ImGui::SliderFloat("Falloff", &fogFalloff, 0.0f, 1000.0f);
		ImGui::ColorEdit3("FogColorAway", glm::value_ptr(fogColorAway));
		ImGui::ColorEdit3("FogColorTowards", glm::value_ptr(fogColorTowards));
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
