#include "RectBlurTap5Shader.h"

namespace bento
{
	void RectBlurTap5Shader::Render(RectangleTexture& _texture, float _offset)
	{
		BindPerPass();
		m_fragmentShader.SetSourceTexture(_texture);
		m_fragmentShader.SetUniform("u_offset", _offset);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}
