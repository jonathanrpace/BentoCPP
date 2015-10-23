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
	m_depthStencilFormat(_depthStencilFormat),
	m_texturesByAttachment(MAX_ATTACHMENTS),
	m_levelsByAttachment(MAX_ATTACHMENTS)
{

}

Kaiga::RenderTargetBase::~RenderTargetBase()
{
	Invalidate();
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

	GL_CHECK(glGenFramebuffers(1, &m_frameBuffer));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));

	GLenum textureTarget = m_isRectangular ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;
	for (int i = 0; i < MAX_ATTACHMENTS; i++ )
	{
		ITexture2D* texture = m_texturesByAttachment[i];
		if (texture == nullptr)
			break;
		int level = m_levelsByAttachment[i];
		texture->SetSize(m_width << level, m_height << level);

	}
}

void Kaiga::RenderTargetBase::OnInvalidate()
{
	if (glIsFramebuffer(m_frameBuffer))
		GL_CHECK(glDeleteFramebuffers(1, &m_frameBuffer));
}

void Kaiga::RenderTargetBase::AttachTexture(int _index, RectangleTexture* _texture, int _level)
{
	assert(m_isRectangular);
	assert(_index < MAX_ATTACHMENTS);
	m_texturesByAttachment[_index] = _texture;
	m_levelsByAttachment[_index] = _index;
}
