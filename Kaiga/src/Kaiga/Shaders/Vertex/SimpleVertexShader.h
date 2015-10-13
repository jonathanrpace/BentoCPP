#pragma once

#include <kaiga.h>

namespace Kaiga
{
	class SimpleVertexShader
		: public AbstractShaderStage
	{
	public:
		SimpleVertexShader()
			: AbstractShaderStage("shaders/vertex/SimpleVertexShader.vert",	GL_VERTEX_SHADER)
		{
		}

		void BindPerModel();
	};
}