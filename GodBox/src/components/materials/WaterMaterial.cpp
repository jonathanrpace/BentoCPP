#include "WaterMaterial.h"

#include <imgui.h>

namespace godBox
{
	WaterMaterial::WaterMaterial(std::string _name)
		: Component(_name, typeid(WaterMaterial))
		, SerializableBase("WaterMaterial")
		, foamTexture("textures/Foam.dds")
	{
		SerializableMember("filterColor",			vec3(0.219f, 0.286f, 0.278f), &filterColor);
		SerializableMember("scatterColor",			vec3(0.219f, 0.286f, 0.278f), &scatterColor);

		SerializableMember("reflectivity",			80.0f, &reflectivity);
		SerializableMember("indexOfRefraction",		0.76f, &indexOfRefraction);
		
		SerializableMember("dirtScalar",			1.0f, &dirtScalar);
		SerializableMember("foamRepeat",			1.0f, &foamRepeat);
		SerializableMember("foamStrength",			1.0f, &foamStrength);

		SerializableMember("depthToFilter",			0.01f, &depthToFilter);
		SerializableMember("depthToDiffuse",		0.01f, &depthToDiffuse);
		SerializableMember("depthToReflection",		0.001f, &depthToReflection);
		SerializableMember("depthPower",			1.00f, &depthPower);
		SerializableMember("localDepthMip",			4.00f, &localDepthMip);
		SerializableMember("localDepthScalar",		1.00f, &localDepthScalar);
		SerializableMember("localDepthValue",		0.01f, &localDepthValue);

		SerializableMember("flowSpeed",				0.02f, &flowSpeed);
		SerializableMember("flowOffset",			0.01f, &flowOffset);
		
		SerializableMember("waveLevels",			3,    &waveLevels);
		SerializableMember("waveAmplitude",			0.1f, &waveAmplitude);
		SerializableMember("waveFrquencyBase",		1.0f, &waveFrquencyBase);
		SerializableMember("waveFrquencyScalar",	2.0f, &waveFrquencyScalar);
		SerializableMember("waveRoughness",			0.5f, &waveRoughness);
		SerializableMember("waveChoppy",			3.0f, &waveChoppy);
		SerializableMember("waveChoppyEach",		0.2f, &waveChoppyEase);
		SerializableMember("waveSpeed",				0.02f, &waveSpeed);

		ResetToDefaults();
	}

	void WaterMaterial::AddUIElements()
	{
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Filter color", glm::value_ptr(filterColor));
		ImGui::ColorEdit3("Scatter color", glm::value_ptr(scatterColor));
		ImGui::Spacing();

		ImGui::SliderFloat("Reflectivity", &reflectivity, 0.0f, 1.0f);
		ImGui::SliderFloat("I.O.R", &indexOfRefraction, 0.5f, 1.5f);
		ImGui::Spacing();

		ImGui::SliderFloat("Dirt scalar", &dirtScalar, 0.0f, 1000.0f);
		ImGui::SliderFloat("Foam repeat", &foamRepeat, 0.0f, 10.0f);
		ImGui::SliderFloat("Foam strength", &foamStrength, 0.0f, 10.0f);
		ImGui::Spacing();

		ImGui::SliderFloat("Depth to reflect", &depthToReflection, 0.0f, 0.01f, "%.5f");
		ImGui::SliderFloat("Depth to filter", &depthToFilter, 0.0f, 0.25f, "%.5f");
		ImGui::SliderFloat("Depth to diffuse", &depthToDiffuse, 0.0f, 0.25f, "%.5f");
		ImGui::SliderFloat("Depth power", &depthPower, 0.1f, 3.0f);
		ImGui::SliderFloat("Local depth mip", &localDepthMip, 0.0f, 8.0f);
		ImGui::SliderFloat("Local depth scalar", &localDepthScalar, 0.0f, 500.0f);
		ImGui::SliderFloat("Local depth value", &localDepthValue, 0.0f, 0.5f, "%.4f");
		ImGui::Spacing();

		ImGui::SliderFloat("Flow speed", &flowSpeed, 0.0f, 1.0f);
		ImGui::SliderFloat("Flow offset", &flowOffset, 0.0f, 0.2f);
		ImGui::Spacing();
		
		ImGui::SliderInt("Levels", &waveLevels, 0, 8);
		ImGui::SliderFloat("Amplitude", &waveAmplitude, 0.0f, 0.01f, "%.5f");
		ImGui::SliderFloat("Freq base", &waveFrquencyBase, 1.0f, 100.0f);
		ImGui::SliderFloat("Freq scalar", &waveFrquencyScalar, 0.0f, 2.0f);
		ImGui::SliderFloat("Roughness", &waveRoughness, 0.0f, 1.0f);
		ImGui::SliderFloat("Choppy", &waveChoppy, 0.0f, 4.0f);
		ImGui::SliderFloat("Choppy ease", &waveChoppyEase, 0.0f, 1.0f);
		ImGui::SliderFloat("Speed", &waveSpeed, 0.0f, 0.2f, "%.4f");
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
