#include "ResolveShader.h"

namespace bento
{
	void ResolveShader::Render
	(
		RectangleTexture & _colorBuffer, 
		RectangleTexture & _blurredColorBuffer,
		ITexture & _dirtyLensTexture,
		float _bloomPower,
		float _bloomStrength
	)
	{
		BindPerPass();
		m_fragmentShader.SetTexture("s_colorBuffer", _colorBuffer);
		m_fragmentShader.SetTexture("s_blurredColorBuffer", _blurredColorBuffer);
		m_fragmentShader.SetTexture("s_dirtyLens", _dirtyLensTexture);
		m_fragmentShader.SetUniform("u_bloomPow", _bloomPower);
		m_fragmentShader.SetUniform("u_bloomStrength", _bloomStrength);
		m_screenQuadGeom.Bind();
		m_screenQuadGeom.Draw();
	}
}


