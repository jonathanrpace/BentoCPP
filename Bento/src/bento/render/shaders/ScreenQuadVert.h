#pragma once

#include <bento/core/ShaderStageBase.h>

namespace bento
{
	struct ScreenQuadVert : ShaderStageBase
	{
		ScreenQuadVert() : ShaderStageBase("shaders/ScreenQuad.vert") {}
	};
}