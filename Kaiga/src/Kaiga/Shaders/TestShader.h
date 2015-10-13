#pragma once

#include <Kaiga/Shaders/Core/AbstractShader.h>
#include <Kaiga/Shaders/Vertex/SimpleVertexShader.h>
#include <Kaiga/Shaders/Fragment/TestFragmentShader.h>

namespace Kaiga
{
	class TestShader :
		public AbstractShader<SimpleVertexShader, TestFragmentShader>
	{

	};
}