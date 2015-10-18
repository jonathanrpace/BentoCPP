#pragma once

#include <kaiga.h>

namespace Kaiga
{
	struct ScreenQuadVert : ShaderStageBase
	{
		ScreenQuadVert() : 	ShaderStageBase("shaders/ScreenQuad.vert") {}
	};
}