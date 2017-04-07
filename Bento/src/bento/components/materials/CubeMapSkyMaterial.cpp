#include "CubeMapSkyMaterial.h"

#include <imgui.h>

namespace bento
{
	CubeMapSkyMaterial::CubeMapSkyMaterial(std::string _name)
		: Component(_name, typeid(CubeMapSkyMaterial))
		, SerializableBase("CubeMapSkyMaterial")
		, envMap("textures/EnvMap.dds")
	{
		// Global
		SerializableMember("intensity", 1.0f, &intensity);
		SerializableMember("mipLevel", 0.0f, &mipLevel);

		ResetToDefaults();
	}

	void CubeMapSkyMaterial::AddUIElements()
	{
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 2.0f);
		ImGui::SliderFloat("mipLevel", &mipLevel, 0.0f, 10.0f);

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
