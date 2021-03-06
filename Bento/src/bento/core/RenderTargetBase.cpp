#include "RenderTargetBase.h"
#include <bento/util/TextureUtil.h>

namespace bento
{
	RenderTargetBase::RenderTargetBase
	(
		int _width,
		int _height,
		bool _isRectangular,
		bool _hasDepthStencil,
		GLenum _depthStencilFormat
	)
		: m_width(_width)
		, m_height(_height)
		, m_frameBuffer(-1)
		, m_depthBuffer(-1)
		, m_isRectangular(_isRectangular)
		, m_hasDepthStencil(_hasDepthStencil)
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
			ITexture* texture = iter.second;
			int level = m_levelsByAttachment[attachment];

			if ( m_isRectangular )
			{
				((RectangleTexture*) texture)->Width(m_width << level);
				((RectangleTexture*) texture)->Height(m_height << level);
			}
			else
			{
				int squareSize = textureUtil::GetBestPowerOfTwo(glm::max(m_width, m_height));
				((TextureSquare*) texture)->Size(squareSize << level);
			}

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->Target(), texture->TextureName(), level));
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
		if (_texture.Size() == m_width && _texture.Size() == m_height)
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

	void RenderTargetBase::SetDrawTextures(TextureSquare & _texture0)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		SetDrawBuffers(drawBuffers, 1);
	}

	void RenderTargetBase::SetDrawTextures(TextureSquare & _texture0, TextureSquare& _texture1)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		SetDrawBuffers(drawBuffers, 2);
	}

	void RenderTargetBase::SetDrawTextures(TextureSquare & _texture0, TextureSquare& _texture1, TextureSquare& _texture2)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		AttachTexture(GL_COLOR_ATTACHMENT2, _texture2);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		SetDrawBuffers(drawBuffers, 3);
	}

	void RenderTargetBase::SetDrawTextures(TextureSquare & _texture0, TextureSquare& _texture1, TextureSquare& _texture2, TextureSquare& _texture3)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		AttachTexture(GL_COLOR_ATTACHMENT2, _texture2);
		AttachTexture(GL_COLOR_ATTACHMENT3, _texture3);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		SetDrawBuffers(drawBuffers, 4);
	}

	void RenderTargetBase::SetDrawTextures(RectangleTexture & _texture0)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		SetDrawBuffers(drawBuffers, 1);
	}

	void RenderTargetBase::SetDrawTextures(RectangleTexture & _texture0, RectangleTexture& _texture1)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		SetDrawBuffers(drawBuffers, 2);
	}

	void RenderTargetBase::SetDrawTextures(RectangleTexture & _texture0, RectangleTexture& _texture1, RectangleTexture& _texture2)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		AttachTexture(GL_COLOR_ATTACHMENT2, _texture2);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		SetDrawBuffers(drawBuffers, 3);
	}

	void RenderTargetBase::SetDrawTextures(RectangleTexture & _texture0, RectangleTexture& _texture1, RectangleTexture& _texture2, RectangleTexture& _texture3)
	{
		AttachTexture(GL_COLOR_ATTACHMENT0, _texture0);
		AttachTexture(GL_COLOR_ATTACHMENT1, _texture1);
		AttachTexture(GL_COLOR_ATTACHMENT2, _texture2);
		AttachTexture(GL_COLOR_ATTACHMENT3, _texture3);
		static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		SetDrawBuffers(drawBuffers, 4);
	}

	/////////////////////////////////////////
	// PROTECTED
	/////////////////////////////////////////
	
	void RenderTargetBase::SetDrawBuffers(const GLenum * const _drawBuffers, GLsizei _size)
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(_size, _drawBuffers));
	}
}