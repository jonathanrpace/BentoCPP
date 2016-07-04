#include "RenderTargetBase.h"

namespace bento
{
	RenderTargetBase::RenderTargetBase
	(
		int _width,
		int _height,
		bool _isRectangular,
		bool _hasDepthStencil,
		GLenum _internalFormat,
		GLenum _depthStencilFormat
	)
		: m_width(_width)
		, m_height(_height)
		, m_frameBuffer(-1)
		, m_depthBuffer(-1)
		, m_isRectangular(_isRectangular)
		, m_hasDepthStencil(_hasDepthStencil)
		, m_internalFormat(_internalFormat)
		, m_depthStencilFormat(_depthStencilFormat)
	{

	}

	RenderTargetBase::~RenderTargetBase()
	{
		Invalidate();
		m_texturesByAttachment.clear();
		m_levelsByAttachment.clear();
	}

	void RenderTargetBase::SetSize(int _width, int _height)
	{
		_width = _width < 16 ? 16 : _width;
		_height = _height < 16 ? 16 : _height;

		if (_width == m_width && _height == m_height)
			return;

		assert(m_isRectangular || _width == _height);

		m_width = _width;
		m_height = _height;
		Invalidate();
	}

	void RenderTargetBase::Bind()
	{
		ValidateNow();
		assert(glIsFramebuffer(m_frameBuffer));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));
	}

	void RenderTargetBase::Validate()
	{
		assert(glIsFramebuffer(m_frameBuffer) == false);

		GL_CHECK(glGenFramebuffers(1, &m_frameBuffer));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer));

		GLenum textureTarget = m_isRectangular ? GL_TEXTURE_RECTANGLE : GL_TEXTURE_2D;
		for (auto& iter : m_texturesByAttachment) 
		{
			GLenum attachment = iter.first;
			ITexture2D& texture = *iter.second;
			int level = m_levelsByAttachment[attachment];
			texture.SetSize(m_width << level, m_height << level);

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, texture.TextureName(), level));
		}

		if (m_hasDepthStencil)
		{
			assert(glIsRenderbuffer(m_depthBuffer) == false);
			GL_CHECK(glGenRenderbuffers(1, &m_depthBuffer));
			GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer));
			GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, m_depthStencilFormat, m_width, m_height));
			GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer));
		}

		// Check status of frame buffer
		{
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				DebugBreak();
			}
		}
	}

	void RenderTargetBase::OnInvalidate()
	{
		if (glIsFramebuffer(m_frameBuffer))
		{
			GL_CHECK(glDeleteFramebuffers(1, &m_frameBuffer));
			m_frameBuffer = -1;
		}

		if (glIsRenderbuffer(m_depthBuffer))
		{
			GL_CHECK(glDeleteRenderbuffers(1, &m_depthBuffer));
			m_depthBuffer = -1;
		}
	}

	void RenderTargetBase::AttachTexture(GLenum _attachment, RectangleTexture& _texture, int _level)
	{
		assert(m_isRectangular);

		// Attach immediately if we're ready to go
		if (_texture.Width() == m_width && _texture.Height() == m_height)
		{
			m_texturesByAttachment[_attachment] = &_texture;
			m_levelsByAttachment[_attachment] = _level;

			Bind();
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, _attachment, GL_TEXTURE_RECTANGLE, _texture.TextureName(), _level));
		}
		else
		{
			m_texturesByAttachment[_attachment] = &_texture;
			m_levelsByAttachment[_attachment] = _level;
			Invalidate();
		}
	}

	void RenderTargetBase::AttachTexture(GLenum _attachment, TextureSquare& _texture, int _level)
	{
		assert(!m_isRectangular);

		// Attach immediately if we're ready to go
		if (_texture.Width() == m_width && _texture.Height() == m_height)
		{
			m_texturesByAttachment[_attachment] = &_texture;
			m_levelsByAttachment[_attachment] = _level;

			Bind();
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, _attachment, GL_TEXTURE_2D, _texture.TextureName(), _level));
		}
		else
		{
			m_texturesByAttachment[_attachment] = &_texture;
			m_levelsByAttachment[_attachment] = _level;
			Invalidate();
		}
	}



	void RenderTargetBase::SetDrawBuffers(GLenum * const _drawBuffers, GLsizei _size)
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(_size, _drawBuffers));
	}
}