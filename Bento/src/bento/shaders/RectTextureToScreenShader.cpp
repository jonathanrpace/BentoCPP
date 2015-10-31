#include "RectTextureToScreenShader.h"

namespace bento
{
	void RectTextureToScreenShader::Render(RectangleTexture * _texture)
	{
		BindPerPass();
		m_fragmentShader.SetSourceTexture(_texture);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}