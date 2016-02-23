#include "TerrainMaterial.h"

#include <imgui.h>

namespace bento
{
	TerrainMaterial::TerrainMaterial(std::string _name)
		: Component(_name, typeid(TerrainMaterial))
		, SomeTexture(256, GL_RGBA8, GL_LINEAR, GL_LINEAR)
		, waterColor(0.37f, 0.73f, 0.69f)
		, waterTranslucentColor(0.37f*0.8f, 0.73f*0.8f, 0.69f*0.8f)
		, lightAltitude((float)-M_PI * 0.5f)
		, lightAzimuth(0.0f)
	{
		SomeTexture.TexImage2D("textures/DataMap.png");
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
		ImGui::SliderFloat("I.O.R", &waterIndexOfRefraction, -2.0f, 2.0f);
		ImGui::ColorEdit3("WaterColor", glm::value_ptr(waterColor));
		ImGui::ColorEdit3("WaterTranslucentColor", glm::value_ptr(waterTranslucentColor));
		ImGui::Text("Waves");
		ImGui::SliderFloat("Effect strength", &waveEffectStrength, 0.0f, 0.2f);
		ImGui::SliderFloat("Effect limit", &waveEffectLimit, 0.0f, 0.5f);
		ImGui::SliderFloat("Choppy limit", &choppyLimit, 0.0f, 0.5f);
		ImGui::SliderFloat("Choppy power", &choppyPower, 0.0f, 3.0f);
		ImGui::Text("Wave Strength");
		ImGui::SliderFloat("Strength 0", &waveStrength0, 0.0f, 1.0f);
		ImGui::SliderFloat("Strength 1", &waveStrength1, 0.0f, 1.0f);
		ImGui::SliderFloat("Strength 2", &waveStrength2, 0.0f, 1.0f);
		ImGui::SliderFloat("Strength 3", &waveStrength3, 0.0f, 1.0f);
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
		ImGui::SliderFloat("Scale 0", &waveScale0, 0.0f, 8.0f);
		ImGui::SliderFloat("Scale 1", &waveScale1, 0.0f, 8.0f);
		ImGui::SliderFloat("Scale 2", &waveScale2, 0.0f, 8.0f);
		ImGui::SliderFloat("Scale 3", &waveScale3, 0.0f, 8.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Lighting");
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 0.0f);
		ImGui::SliderFloat("DirectIntensity", &directLightIntensity, 0.0f, 2.0f);
		ImGui::SliderFloat("AmbientIntensity", &ambientLightIntensity, 0.0f, 2.0f);
		ImGui::Spacing();
	}

}
