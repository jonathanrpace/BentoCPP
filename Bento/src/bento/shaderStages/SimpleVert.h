#pragma once

#include <bento/core/ShaderStageBase.h>

namespace bento
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