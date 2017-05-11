#include "SmokeParticleMaterial.h"

#include <imgui.h>

namespace godBox
{
	SmokeParticleMaterial::SmokeParticleMaterial(std::string _name)
		: Component(_name, typeid(SmokeParticleMaterial))
		, SerializableBase("SmokeParticleMaterial")
		, texture("textures/SmokeParticle.dds")
	{
		SERIALIZABLE(baseColor, vec3(0.8f, 0.8f, 0.8f));
		SERIALIZABLE(filterColor, vec3(0.5f, 0.5f, 0.5f));

		SERIALIZABLE(density, 1.0f);
		SERIALIZABLE(filterStrength, 1.0f);

		ResetToDefaults();
	}

	void SmokeParticleMaterial::AddUIElements()
	{
		ImGui::SliderFloat("Density", &density, 0.0f, 5.0f);
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);
		ImGui::ColorEdit3("Base color", glm::value_ptr(baseColor));
		ImGui::Spacing();
		ImGui::ColorEdit3("Filter color", glm::value_ptr(filterColor));
		ImGui::SliderFloat("Filter strength", &filterStrength, 0.0f, 10.0f);
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
