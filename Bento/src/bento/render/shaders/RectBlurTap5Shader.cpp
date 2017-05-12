#include "RectBlurTap5Shader.h"

namespace bento
{
	void RectBlurTap5Shader::Render(RectangleTexture& _texture)
	{
		BindPerPass();
		m_fragmentShader.SetSourceTexture(_texture);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}
