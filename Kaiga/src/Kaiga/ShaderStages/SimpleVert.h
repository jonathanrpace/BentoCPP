#pragma once

#include <Kaiga/Core/ShaderStageBase.h>

namespace Kaiga
{
	struct SimpleVert : ShaderStageBase
	{
		SimpleVert() : ShaderStageBase("shaders/Simple.vert") {}

		void BindPerModel()
		{
			SetUniform("mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		}
	};
}