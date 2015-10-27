#pragma once

#include <kaiga.h>

namespace Kaiga
{
	struct TestFrag : ShaderStageBase
	{
		TestFrag() : ShaderStageBase("shaders/Test.frag") {}
	};
}