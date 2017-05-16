#include "ResolveShader.h"

namespace bento
{
	void ResolveShader::Render(RectangleTexture & _colorBuffer, RectangleTexture & _blurredColorBuffer)
	{
		BindPerPass();
		m_fragmentShader.SetTexture("s_colorBuffer", _colorBuffer);
		m_fragmentShader.SetTexture("s_blurredColorBuffer", _blurredColorBuffer);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}


