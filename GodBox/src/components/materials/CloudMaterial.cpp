#include "CloudMaterial.h"

#include <imgui.h>

namespace godBox
{
	CloudMaterial::CloudMaterial(std::string _name)
		: Component(_name, typeid(CloudMaterial))
		, SerializableBase("CloudMaterial")
		, baseShapesTexture3D("textures/CloudBaseShapes3D.dds")
		, detailTexture3D("textures/CloudDetailMap3D.dds")
	{
		SERIALIZABLE(maxRayLength, 10.0f);
		SERIALIZABLE(coarseStepsPerUnit, 16);
		SERIALIZABLE(detailStepsPerUnit, 32);
		SERIALIZABLE(lightConeMaxLength, 0.1f);

		SERIALIZABLE(vortexTurns, 0.0f);

		SERIALIZABLE(baseScale, 1.0f);
		SERIALIZABLE(baseScaleVertical, 1.0f);
		SERIALIZABLE(detailScale, 4.0f);
		SERIALIZABLE(detailScaleVertical, 1.0f);
		SERIALIZABLE(detailMaxDensity, 0.2f);

		SERIALIZABLE(position, 1.0f);
		SERIALIZABLE(height, 1.0f);
		SERIALIZABLE(falloffTop, 0.05f);
		SERIALIZABLE(falloffBottom, 0.05f);

		SERIALIZABLE(lightAzimuth, 0.0f);
		SERIALIZABLE(lightAltitude, (float)-M_PI * 0.5f);
		SERIALIZABLE(lightIntensity, 1.0f);
		SERIALIZABLE(lightColor, vec3(1.0,1.0,1.0));
		SERIALIZABLE(skyColor, vec3(1.0,1.0,1.0));

		SERIALIZABLE(absorbtion, 1.0f);
		SERIALIZABLE(powderSharpness, 1-.0f);
		SERIALIZABLE(baseDensityScalar, 1.0f);
		SERIALIZABLE(baseDensityOffset, 0.0f);
		SERIALIZABLE(scatteringParam, 1.0f);

		SERIALIZABLE(detailDensityScalar, 1.0f);
		SERIALIZABLE(detailDensityOffset, 0.0f);

		ResetToDefaults();
	}

	void CloudMaterial::AddUIElements()
	{
		ImGui::ColorEditMode(ImGuiColorEditMode_HSV);

		ImGui::Spacing();
		ImGui::Text("Transform");
		ImGui::DragFloat("Base scale", &baseScale, 0.01f, 0.0f );
		ImGui::DragFloat("Base scale vertical", &baseScaleVertical, 0.01f, 0.0f );
		ImGui::DragFloat("Detail scale", &detailScale, 0.01f, 0.0f );
		ImGui::DragFloat("Detail scale vertical", &detailScaleVertical, 0.01f, 0.0f );
		ImGui::DragFloat("Position", &position, 0.01f );
		ImGui::DragFloat("Height", &height, 0.01f, 0.0f );
		ImGui::DragFloat("Falloff top", &falloffTop, 0.01f, 0.0f );
		ImGui::DragFloat("Falloff bottom", &falloffBottom, 0.01f, 0.0f );
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Vortex");
		ImGui::DragFloat("Num turns", &vortexTurns, 0.01f );
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Quality");
		ImGui::DragFloat("Max ray length", &maxRayLength, 0.01f, 0.0f, 10.0f);
		ImGui::DragInt("Coarse steps per unit", &coarseStepsPerUnit, 1.0f, 1, 256);
		ImGui::DragInt("Detail steps per unit", &detailStepsPerUnit, 1.0f, 1, 512);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Lighting");
		ImGui::SliderAngle("Azimuth", &lightAzimuth, -180.0f, 180.0f);
		ImGui::SliderAngle("Altitude", &lightAltitude, -180.0f, 180.0f);
		ImGui::DragFloat("Intensity", &lightIntensity, 0.01f);
		ImGui::ColorEdit3("Color", glm::value_ptr(lightColor));
		ImGui::ColorEdit3("Sky Color", glm::value_ptr(skyColor));
		ImGui::DragFloat("Light cone scale", &lightConeMaxLength, 0.001f, 0.0f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Properties");
		ImGui::DragFloat("Absorbtion", &absorbtion, 0.01f, 0.0f);
		ImGui::DragFloat("PowderSharpness", &powderSharpness, 0.1f, 0.0f);
		ImGui::DragFloat("Base Density Scale", &baseDensityScalar, 0.1f, 0.0f);
		ImGui::DragFloat("Base Density Offset", &baseDensityOffset, 0.01f);
		ImGui::DragFloat("ScatteringParam", &scatteringParam, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Detail Range", &detailMaxDensity, 0.01f, 0.0f);
		ImGui::DragFloat("Detail Density Scale", &detailDensityScalar, 0.1f, 0.0f);
		ImGui::DragFloat("Detail Density Offset", &detailDensityOffset, 0.01f);

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
