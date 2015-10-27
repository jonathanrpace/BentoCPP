#pragma once

#include <Kaiga/Core/ShaderStageBase.h>

namespace Kaiga
{
	struct ScreenQuadVert : ShaderStageBase
	{
		ScreenQuadVert() : ShaderStageBase("shaders/ScreenQuad.vert") {}
	};
}