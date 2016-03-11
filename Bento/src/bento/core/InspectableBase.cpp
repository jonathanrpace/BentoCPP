#include "InspectableBase.h"

#include <imgui.h>
#include <bento/core/DefaultsManager.h>

namespace bento
{
	InspectableBase::InspectableBase(char * const _namespace)
		: m_namespace(_namespace)
		, m_inspectableNames()
		, m_inspectableTypes()
		, m_sliderParams()
		, m_numInspectables(0)
	{
	}

	InspectableBase::~InspectableBase()
	{
		m_inspectableNames.clear();
		m_inspectableTypes.clear();
		m_sliderParams.clear();
	}

	void InspectableBase::RegisterInspectableMemberSlider(char const * _name, float _default, float _min, float _max, float* _ptr)
	{
		m_inspectableNames[m_numInspectables] = _name;
		m_inspectableTypes[m_numInspectables] = eInspectableType_slider;
		SliderParams params;
		params.default = _default;
		params.min = _min;
		params.max = _max;
		params.ptr = _ptr;
		m_sliderParams[m_numInspectables] = params;

		m_numInspectables++;
	}

	void InspectableBase::AddUIElements()
	{
		for (int i = 0; i < m_numInspectables; i++)
		{
			char const * name = m_inspectableNames[i];
			InspectableType type = m_inspectableTypes[i];

			switch (type)
			{
			case eInspectableType_slider :
				SliderParams params = m_sliderParams[i];
				ImGui::SliderFloat(name, params.ptr, params.min, params.max);
				break;
			}
		}

		if (ImGui::Button("Save"))
		{
			DefaultsManager::Flush();
		}

		if (ImGui::Button("Revert"))
		{
			ResetInspectableMembersToDefaults();
		}
	}

	void InspectableBase::ResetInspectableMembersToDefaults()
	{
		DefaultsManager::SetNamespace(m_namespace);

		for (int i = 0; i < m_numInspectables; i++)
		{
			char const * name = m_inspectableNames[i];
			InspectableType type = m_inspectableTypes[i];

			switch (type)
			{
				case eInspectableType_slider:
					SliderParams params = m_sliderParams[i];
					DefaultsManager::GetValue(name, params.default, params.ptr);
					break;
			}
		}
	}

}