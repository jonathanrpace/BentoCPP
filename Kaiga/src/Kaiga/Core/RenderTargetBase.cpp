#include "RenderTargetBase.h"

#include <kaiga.h>
#include <assert.h>

Kaiga::RenderTargetBase::RenderTargetBase
(
	int _width,
	int _height,
	bool _isRectangular,
	bool _hasDepthStencil,
	GLenum _internalFormat,
	GLenum _depthStencilFormat
) :
	m_width(_width),
	m_height(_height),
	m_isRectangular(_isRectangular),
	m_hasDepthStencil(_hasDepthStencil),
	m_internalFormat(_internalFormat),
	m_depthStencilFormat(_depthStencilFormat)
{

}

Kaiga::RenderTargetBase::~RenderTargetBase()
{
}

void Kaiga::RenderTargetBase::SetSize(int _width, int _height)
{
	if (_width == m_width && _height == m_height)
		return;

	assert( m_isRectangular || _width == _height);

	m_width = _width;
	m_height = _height;
	Invalidate();
}

void Kaiga::RenderTargetBase::Bind()
{
	Validate();
	GL_CHECK(glBindFramebuffer(m_frameBuffer, GL_FRAMEBUFFER));
}

void Kaiga::RenderTargetBase::Validate()
{
	assert(glIsFramebuffer(m_frameBuffer) == false);

	glGenFramebuffers(1, &m_frameBuffer);
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));

	GLenum textureTarget = m_isRectangular ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;

}

void Kaiga::RenderTargetBase::OnInvalidate()
{
	if (glIsFramebuffer(m_frameBuffer))
	{
		GL_CHECK(glDeleteFramebuffers(1, &m_frameBuffer));
	}
}
