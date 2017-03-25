#include "TerrainMaterial.h"

#include <imgui.h>

namespace godBox
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SerializableBase("TerrainMaterial")
		, grungeTexture("textures/GrungeMap.dds", GL_LINEAR, GL_LINEAR)
		//, smokeTexture("textures/Smoke.png")
		, foamTexture("textures/Foam.dds")
		, lavaAlb("textures/Lava_Albedo.dds")
		, lavaNrm("textures/Lava_Normal.dds")
		, lavaMat("textures/Lava_Material.dds")
		, lavaLongAlb("textures/LavaLong_Albedo.dds")
		, lavaLongNrm("textures/LavaLong_Normal.dds")
		, lavaLongMat("textures/LavaLong_Material.dds")
		, lavaLatAlb("textures/LavaLat_Albedo.dds")
		, lavaLatNrm("textures/LavaLat_Normal.dds")
		, lavaLatMat("textures/LavaLat_Material.dds")
	{
		// Global
		SerializableMember("heightScalar", 0.0f, &heightOffset);

		// Rock
		SerializableMember("rockReflectivity", 0.1f, &rockReflectivity);
		SerializableMember("rockDetailBumpStrength", 1.0f, &rockDetailBumpStrength);
		SerializableMember("rockDetailBumpSlopePower", 1.0f, &rockDetailBumpSlopePower);

		// Creases
		SerializableMember("bearingCreaseScalar", 1.0f, &bearingCreaseScalar);
		SerializableMember("lateralCreaseScalar", 1.0f, &lateralCreaseScalar);
		SerializableMember("creaseRatio", 0.1f, &creaseRatio);
		SerializableMember("creaseMipLevel", 0.0f, &creaseMipLevel);
		SerializableMember("creaseForwardScalar", 1.0f, &creaseForwardScalar);
		SerializableMember("creaseMapRepeat", 1.0f, &creaseMapRepeat);
		SerializableMember("creaseGridRepeat", 1.0f, &creaseGridRepeat);
		SerializableMember("moltenFlowSpeed", 0.02f, &moltenFlowSpeed);
		SerializableMember("moltenFlowOffset", 0.01f, &moltenFlowOffset);

		// Molten
		SerializableMember("moltenColor", vec3(1.0f, 0.5f, 0.01f), &moltenColor);
		SerializableMember("moltemColorScalar", 1.5f, &moltenColorScalar);
		SerializableMember("moltenAlphaScalar", 1.0f, &moltenAlphaScalar);
		SerializableMember("moltenAlphaPower", 1.0f, &moltenAlphaPower);

		// Glow
		SerializableMember("glowScalar", 1.0f, &glowScalar);

		// Water
		SerializableMember("waterColor", vec3(0.219f, 0.286f, 0.278f), &waterColor);
		SerializableMember("waterSpecularPower", 80.0f, &waterSpecularPower);
		SerializableMember("waterIndexOfRefraction", 0.33f, &waterIndexOfRefraction);
		SerializableMember("waterDepthToReflection", 0.05f, &waterDepthToReflect);
		SerializableMember("waterDepthToFilter", 0.05f, &waterDepthToFilter);
		SerializableMember("waterDepthToDiffuse", 0.05f, &waterDepthToDiffuse);
		SerializableMember("dissolvedDirtDesnsityScalar", 1.0f, &dissolvedDirtDesntiyScalar);
		SerializableMember("waterFlowSpeed", 0.02f, &waterFlowSpeed);
		SerializableMember("waterFlowOffset", 0.01f, &waterFlowOffset);
		SerializableMember("waterFlowRepeat", 1.0f, &waterFlowRepeat);
		SerializableMember("waterWaveSpeed", 0.02f, &waterWaveSpeed);

		// Foam
		SerializableMember("foamReat", 1.0f, &foamRepeat);
		SerializableMember("foamDistortStrength", 0.1f, &foamDistortStrength);
		SerializableMember("foamAlphaStrength", 1.0f, &foamAlphaStrength);

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
		ImGui::SliderFloat("Height Offset", &heightOffset, 0.0f, 0.1f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::SliderFloat("Reflectivity A##rock", &rockReflectivity, 0.0f, 1.0f);
		ImGui::SliderFloat("Detail Bump Strength B##rock", &rockDetailBumpStrength, 0.0f, 2.0f);
		ImGui::SliderFloat("Detail Bump Slope Power B##rock", &rockDetailBumpSlopePower, 0.0f, 20.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Creases");
		ImGui::SliderFloat("Bearing Scalar#crease", &bearingCreaseScalar, 0.0f, 10.0f);
		ImGui::SliderFloat("Lateral Scalar#crease", &lateralCreaseScalar, 0.0f, 10.0f);
		ImGui::SliderFloat("Ratio##crease", &creaseRatio, -1.5f, 1.5f);
		ImGui::SliderFloat("MipLevel##crease", &creaseMipLevel, 0.0f, 8.0f);
		ImGui::SliderFloat("Forward Scalar##crease", &creaseForwardScalar, 0.0f, 0.1f);
		ImGui::SliderFloat("Map Repeat##crease", &creaseMapRepeat, 0.0f, 10.0f);
		ImGui::SliderFloat("Grid Repeat##crease", &creaseGridRepeat, 0.0f, 50.0f);
		
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Molten");
		ImGui::SliderFloat("Flow Speed##molten", &moltenFlowSpeed, 0.0f, 0.5f);
		ImGui::SliderFloat("Flow Offset##molten", &moltenFlowOffset, 0.0f, 0.50f);
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color##molten", glm::value_ptr(moltenColor));
		ImGui::SliderFloat("Color Scale##molten", &moltenColorScalar, 1.0f, 20.0f);
		ImGui::SliderFloat("Alpha Scale##molten", &moltenAlphaScalar, 0.0f, 10.0f);
		ImGui::SliderFloat("Alpha Power##molten", &moltenAlphaPower, 0.0f, 4.0f);

		ImGui::Spacing();
		ImGui::Text("Glow");
		ImGui::SliderFloat("Scale##glow", &glowScalar, 0.0f, 50.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Water");
		ImGui::SliderFloat("FlowSpeed##water", &waterFlowSpeed, 0.0f, 1.0f);
		ImGui::SliderFloat("FlowOffset##water", &waterFlowOffset, 0.0f, 5.0f);
		ImGui::SliderFloat("FlowRepeat", &waterFlowRepeat, 1.0f, 10.0f);
		ImGui::SliderFloat("SpecularPower", &waterSpecularPower, 0.0f, 1.0f);
		ImGui::SliderFloat("I.O.R", &waterIndexOfRefraction, 0.95f, 1.05f);
		ImGui::ColorEdit3("Color##water", glm::value_ptr(waterColor));
		ImGui::SliderFloat("DepthToReflect", &waterDepthToReflect, 0.0f, 0.01f, "%.4f");
		ImGui::SliderFloat("DepthToFilter", &waterDepthToFilter, 0.0f, 0.1f);
		ImGui::SliderFloat("DepthToDiffuse", &waterDepthToDiffuse, 0.0f, 0.1f);
		ImGui::SliderFloat("DissolvedDirtDesntityScalar", &dissolvedDirtDesntiyScalar, 0.0f, 1000.0f);
		ImGui::SliderFloat("WaveSpeed", &waterWaveSpeed, 0.0f, 0.1f, "%.4f");
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Foam");
		ImGui::SliderFloat("Repeat##foam", &foamRepeat, 0.0f, 10.0f);
		ImGui::SliderFloat("Distortion##foam", &foamDistortStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("AlphaStrength##foam", &foamAlphaStrength, 0.0f, 10.0f);
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
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 2.0f);
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
