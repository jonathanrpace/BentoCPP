#include "TerrainMaterial.h"

#include <imgui.h>

namespace godBox
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SerializableBase("TerrainMaterial")
		, grungeTexture("textures/Lava_Material.dds", GL_LINEAR, GL_LINEAR)
		, lavaAlb("textures/Lava_Albedo.dds")
		, lavaNrm("textures/Lava_Normal.dds", GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR)
		, lavaMat("textures/Lava_Material.dds")
		//, lavaLongAlb("textures/LavaLong_Albedo.dds")
		//, lavaLongNrm("textures/LavaLong_Normal.dds")
		//, lavaLongMat("textures/LavaLong_Material.dds")
		//, lavaLatAlb("textures/LavaLat_Albedo.dds")
		//, lavaLatNrm("textures/LavaLat_Normal.dds")
		//, lavaLatMat("textures/LavaLat_Material.dds")
		, envMap("textures/EnvMap2.dds")
		, irrMap("textures/IrrMap2.dds")
		, moltenGradient("textures/MoltenGradient.dds", GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP)
		, albedoFluidGradient("textures/AlbedoFluidGradient.dds", GL_LINEAR, GL_LINEAR)
	{
		// Global
		SerializableMember("heightScalar", 0.0f, &heightOffset);
		SerializableMember("uvRepeat", 1.0f, &uvRepeat);

		// Rock
		SerializableMember("rockReflectivity", 0.1f, &rockReflectivity);
		SerializableMember("rockDetailBumpStrength", 1.0f, &rockNormalStrength);
		SerializableMember("rockDetailBumpSlopePower", 1.0f, &rockDetailBumpSlopePower);

		// Creases
		SerializableMember("moltenFlowSpeed", 0.02f, &moltenFlowSpeed);
		SerializableMember("moltenFlowOffset", 0.01f, &moltenFlowOffset);
		SerializableMember("smudgeUVStrength", 0.01f, &smudgeUVStrength);
		SerializableMember("creaseSampleOffset", 0.01f, &creaseSampleOffset);
		SerializableMember("creaseSampleMip", 0.0f, &creaseSampleMip);
		SerializableMember("creaseFrequency", 200.0f, &creaseFrequency);
		SerializableMember("creaseNormalStrength", 1.0f, &creaseNormalStrength);
		SerializableMember("creaseDistortStrength", 0.05f, &creaseDistortStrength);
		
		// Glow
		SerializableMember("glowScalar", 1.0f, &glowScalar);

		// Dirt
		SerializableMember("dirtColor", vec3(0.5, 0.5f, 0.5f), &dirtColor);
		SerializableMember("dirtTextureRepeat", 1.0, &dirtTextureRepeat);
		SerializableMember("dirtHeightToOpaque", 1.0, &dirtHeightToOpaque);

		// Lighting
		SerializableMember("lightAzimuth", 0.0f, &lightAzimuth);
		SerializableMember("lightAltitude", (float)-M_PI * 0.5f, &lightAltitude);
		SerializableMember("lightDistance", 1.0f, &lightDistance);
		SerializableMember("directLightIntensity", 1.0f, &directLightIntensity);
		SerializableMember("ambientLightIntensity", 1.2f, &ambientLightIntensity);

		// Fog
		SerializableMember("fogColorAway", vec3(0.7f, 0.7f, 0.7f), &fogColorAway);
		SerializableMember("fogColorTowards", vec3(1.0f, 0.9f, 0.7f), &fogColorTowards);
		SerializableMember("fogDensity", 0.2f, &fogDensity);
		SerializableMember("fogHeight", 0.2f, &fogHeight);

		ResetToDefaults();
	}

	void TerrainMaterial::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Global");
		ImGui::SliderFloat("Height Offset", &heightOffset, 0.0f, 0.05f);
		ImGui::SliderFloat("UV Repeat", &uvRepeat, 0.0f, 4.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::SliderFloat("Reflectivity A##rock", &rockReflectivity, 0.0f, 1.0f);
		ImGui::SliderFloat("Normal Strength B##rock", &rockNormalStrength, 0.0f, 2.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Creases");
		ImGui::SliderFloat("Smudge UV Strength", &smudgeUVStrength, 0.0f, 0.1f);
		ImGui::SliderFloat("Sample Offset##creases", &creaseSampleOffset, 0.0f, 10.0f);
		ImGui::SliderFloat("Sample Mip##creases", &creaseSampleMip, 0.0f, 8.0f);
		ImGui::SliderFloat("Crease Frequency", &creaseFrequency, 0.0f, 1500.0f);
		ImGui::SliderFloat("Normal Strength##creases", &creaseNormalStrength, 0.0f, 2.0f);
		ImGui::SliderFloat("Distort Strength##creases", &creaseDistortStrength, 0.0f, 0.1f);
		ImGui::Spacing();
		
		ImGui::Spacing();
		ImGui::Text("Molten");
		ImGui::SliderFloat("Flow Speed##molten", &moltenFlowSpeed, 0.0f, 0.5f);
		ImGui::SliderFloat("Flow Offset##molten", &moltenFlowOffset, 0.0f, 0.50f);

		ImGui::Spacing();
		ImGui::Text("Glow");
		ImGui::SliderFloat("Scale##glow", &glowScalar, 0.0f, 50.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Dirt");
		ImGui::ColorEdit3("Color##dirt", glm::value_ptr(dirtColor));
		ImGui::SliderFloat("TextureRepeat##dirt", &dirtTextureRepeat, 0.0f, 20.0f);
		ImGui::SliderFloat("HeightToOpaque##dirt", &dirtHeightToOpaque, 0.0f, 0.1f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Lighting");
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 0.0f);
		ImGui::SliderFloat("Distance", &lightDistance, 0.0f, 10.0f);
		ImGui::SliderFloat("DirectIntensity", &directLightIntensity, 0.0f, 4.0f);
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 4.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Fog");
		ImGui::SliderFloat("Density", &fogDensity, 0.0f, 10.0f);
		ImGui::SliderFloat("Height", &fogHeight, 0.0f, 1.0f);
		ImGui::ColorEdit3("ColorAway##fog", glm::value_ptr(fogColorAway));
		ImGui::ColorEdit3("ColorToward##fog", glm::value_ptr(fogColorTowards));
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
