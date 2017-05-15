#include "RectTextureToScreenShader.h"

namespace bento
{
	void RectTextureToScreenShader::Render(RectangleTexture& _texture, float _offset)
	{
		BindPerPass();
		m_fragmentShader.SetSourceTexture(_texture);
		m_fragmentShader.SetUniform("u_offset", _offset);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}
