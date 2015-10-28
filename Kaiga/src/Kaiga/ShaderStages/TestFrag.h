#pragma once

#include <Kaiga\Core\ShaderStageBase.h>

namespace Kaiga
{
	struct TestFrag : ShaderStageBase
	{
		TestFrag() : ShaderStageBase("shaders/Test.frag") {}
	};
}