#pragma once

#include <Kaiga/Shaders/Core/AbstractShaderStage.h>

namespace Kaiga
{
	class TestFragmentShader :
		public AbstractShaderStage
	{
	public:
		TestFragmentShader() 
			: AbstractShaderStage("shaders/fragment/TestShader.frag", GL_FRAGMENT_SHADER)
		{
		}
	};
}