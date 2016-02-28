#pragma once

#include <bento/core/ShaderStageBase.h>

namespace bento
{
	struct NullFrag : ShaderStageBase
	{
		NullFrag() : ShaderStageBase("shaders/Null.frag") {}
	};
}