#pragma once

#include <Kaiga/Shaders/Core/AbstractShaderStage.h>

namespace Kaiga
{
	class ScreenQuadVertexShader :
		public AbstractShaderStage
	{
	public:
		ScreenQuadVertexShader() :
			AbstractShaderStage(
				"shaders/vertex/ScreenQuadShader.vert",
				GL_VERTEX_SHADER)
		{
		}

		virtual void BindPerPass() override
		{
			
		}
	};
}