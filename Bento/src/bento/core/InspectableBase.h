#pragma once

#include <vector>
#include <bento/core/IInspectable.h>

namespace bento
{
	struct SliderParams
	{
		float default;
		float min;
		float max;
		float* ptr;
	};

	enum InspectableType
	{
		eInspectableType_slider
	};

	class InspectableBase : public IInspectable
	{
	public:
		InspectableBase(char * const _namespace);
		~InspectableBase();

	protected:
		void RegisterInspectableMemberSlider(char const * _name, float _default, float _min, float _max, float* _ptr);
		void ResetInspectableMembersToDefaults();

		// Inherited via IInspectable
		virtual void AddUIElements() override;

	private:
		char * const m_namespace;
		std::vector<char const *> m_inspectableNames;
		std::vector<InspectableType> m_inspectableTypes;
		std::vector<SliderParams> m_sliderParams;
		int m_numInspectables;
	};
}