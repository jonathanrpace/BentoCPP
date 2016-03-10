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
		InspectableBase();
		~InspectableBase();

	protected:

		std::vector<char const *> m_inspectableNames;
		std::vector<InspectableType> m_inspectableTypes;
		std::vector<SliderParams> m_sliderParams;
		int m_numInspectables;

		void RegisterInspectableMemberSlider(char const * _name, float _default, float _min, float _max, float* _ptr);

		// Inherited via IInspectable
		virtual void AddUIElements() override;
	};
}