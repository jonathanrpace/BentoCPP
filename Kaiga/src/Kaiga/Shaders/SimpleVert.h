#pragma once

#include <kaiga.h>

namespace Kaiga
{
	struct SimpleVert : ShaderStageBase
	{
		SimpleVert() : ShaderStageBase("shaders/Simple.vert") {}

		void BindPerModel()
		{
			SetUniformMatrix("mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		}
	};
}