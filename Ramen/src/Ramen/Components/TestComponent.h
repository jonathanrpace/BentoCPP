#pragma once

#include <Ramen/Components/ComponentBase.h>

namespace Ramen
{
	class TestComponent : public ComponentBase<TestComponent>
	{
	public:
		char* name;
		float value;
		bool visible;
	};
}

