#include "TerrainMaterial.h"

#include <imgui.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SerializableBase("TerrainMaterial")
		, someTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT)
		, moltenPlateDetailTexture(16, GL_RGBA8, GL_LINEAR, GL_NEAREST, GL_CLAMP, GL_CLAMP)
		, smokeTexture(16, GL_RGBA8, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT)
		, foamTexture(16, GL_RGBA8, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT)
	{
		// Textures
		someTexture.TexImage2D("textures/DataMap.png");
		moltenPlateDetailTexture.TexImage2D("textures/MoltenPlateDetail.png");
		rockDiffuseTexture.TexImage2D("textures/Rock_Diff.png");
		foamTexture.TexImage2D("textures/Foam.png");
		smokeTexture.TexImage2D("textures/Smoke.png");

		// Rock
		SerializableMember("rockColorA", vec3(0.1f, 0.1f, 0.1f), &rockColorA);
		SerializableMember("rockColorB", vec3(0.1f, 0.1f, 0.1f), &rockColorB);
		SerializableMember("rockRoughnessA", 0.1f, &rockRoughnessA);
		SerializableMember("rockRoughnessB", 0.0f, &rockRoughnessB);
		SerializableMember("rockFresnelA", 1.0f, &rockFresnelA);
		SerializableMember("rockFresnelB", 1.0f, &rockFresnelB);
		SerializableMember("rockDetailBumpStrength", 1.0f, &rockDetailBumpStrength);
		SerializableMember("rockDetailDiffuseStrength", 1.0f, &rockDetailDiffuseStrength);

		// Creases
		SerializableMember("bearingCreaseScalar", 1.0f, &bearingCreaseScalar);
		SerializableMember("lateralCreaseScalar", 1.0f, &lateralCreaseScalar);
		SerializableMember("creaseMipLevel", 0.0f, &creaseMipLevel);
		SerializableMember("creaseForwardScalar", 1.0f, &creaseForwardScalar);

		// Hot rock
		SerializableMember("hotRockColor", vec3(0.1f, 0.1f, 0.1f), &hotRockColor);
		SerializableMember("hotRockRoughness", 0.0f, &hotRockRoughness);
		SerializableMember("hotRockFresnel", 1.0f, &hotRockFresnel);

		// Molten
		SerializableMember("moltenColor", vec3(1.0f, 0.5f, 0.01f), &moltenColor);
		SerializableMember("moltenMapAlphaScalar", 1.0f, &moltenMapAlphaScalar);
		SerializableMember("moltenMapAlphaPower", 1.0f, &moltenMapAlphaPower);
		SerializableMember("moltenMapOffset", 0.0f, &moltenMapOffset);

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
		ImGui::Text("Rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color A##rock", glm::value_ptr(rockColorA));
		ImGui::ColorEdit3("Color B##rock", glm::value_ptr(rockColorB));
		ImGui::SliderFloat("Roughness A##rock", &rockRoughnessA, 0.0f, 1.0f);
		ImGui::SliderFloat("Roughness B##rock", &rockRoughnessB, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel A##rock", &rockFresnelA, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel B##rock", &rockFresnelB, 0.0f, 1.0f);
		ImGui::SliderFloat("Detail Bump Strength B##rock", &rockDetailBumpStrength, 0.0f, 100.0f);
		ImGui::SliderFloat("Detail Diffuse Strength B##rock", &rockDetailDiffuseStrength, 0.0f, 1.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Hot rock");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color##hotRock", glm::value_ptr(hotRockColor));
		ImGui::SliderFloat("Roughness##hotRock", &hotRockRoughness, 0.0f, 1.0f);
		ImGui::SliderFloat("Fresnel##hotRock", &hotRockFresnel, 0.0f, 1.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Creases");
		ImGui::SliderFloat("BearingScalar", &bearingCreaseScalar, 0.0f, 1000.0f);
		ImGui::SliderFloat("LateralScalar", &lateralCreaseScalar, 0.0f, 1000.0f);
		ImGui::SliderFloat("MipLevel##crease", &creaseMipLevel, 0.0f, 8.0f);
		ImGui::SliderFloat("ForwardScalar##crease", &creaseForwardScalar, 0.0f, 1.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Molten");
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Color##molten", glm::value_ptr(moltenColor));
		ImGui::SliderFloat("Alpha Scale##molten", &moltenMapAlphaScalar, 0.0f, 8.0f);
		ImGui::SliderFloat("Alpha Power##molten", &moltenMapAlphaPower, 0.0f, 8.0f);
		ImGui::SliderFloat("Offset##molten", &moltenMapOffset, 0.0f, 0.5f);

		ImGui::Spacing();
		ImGui::Text("Water");
		ImGui::SliderFloat("FlowSpeed", &waterFlowSpeed, 0.0f, 1.0f);
		ImGui::SliderFloat("FlowOffset", &waterFlowOffset, 0.0f, 5.0f);
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
