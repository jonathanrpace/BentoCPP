#pragma once

#include <bento/core/ShaderStageBase.h>

namespace bento
{
	struct TestFrag : ShaderStageBase
	{
		TestFrag() : ShaderStageBase("shaders/Test.frag") {}
	};
}