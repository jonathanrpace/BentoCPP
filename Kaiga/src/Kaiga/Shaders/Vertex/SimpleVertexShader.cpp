#include "SimpleVertexShader.h"

namespace Kaiga
{
	void SimpleVertexShader::BindPerModel()
	{
		SetUniformMatrix("mvpMatrix", RenderParams::ModelViewProjectionMatrix());
	}
}
