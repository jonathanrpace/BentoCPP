#include "RenderTarget.h"

#include <bento.h>

namespace godBox
{
	const GLuint RenderTarget::FRAME_BUFFER_ATTACHMENT_POSITION = GL_COLOR_ATTACHMENT0;
	const GLuint RenderTarget::FRAME_BUFFER_ATTACHMENT_COLOR = GL_COLOR_ATTACHMENT1;
	const GLuint RenderTarget::FRAME_BUFFER_ATTACHMENT_COLOR_POST_TRANSPARENCY = GL_COLOR_ATTACHMENT2;
	
	RenderTarget::RenderTarget
	(
		int _width, 
		int _height
	)
		: RenderTargetBase(_width, _height, true)
		, m_positionTexture(_width, _height, GL_RGBA16F, GL_NEAREST, GL_NEAREST)
		, m_colorTexture(_width, _height, GL_RGBA16F)
		, m_colorPostTransparencyTexture(_width, _height, GL_RGBA16F)
	{
		AttachTexture(FRAME_BUFFER_ATTACHMENT_POSITION, m_positionTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_COLOR, m_colorTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_COLOR_POST_TRANSPARENCY, m_colorPostTransparencyTexture);
	}

	RenderTarget::~RenderTarget()
	{
		
	}

	void RenderTarget::Clear()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_POSITION,
			FRAME_BUFFER_ATTACHMENT_COLOR,
			FRAME_BUFFER_ATTACHMENT_COLOR_POST_TRANSPARENCY
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClearDepth(1.0f));
		GL_CHECK(glClearStencil(0));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	}

	void RenderTarget::BindForForwardPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_POSITION,
			FRAME_BUFFER_ATTACHMENT_COLOR
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	void RenderTarget::BindForTransparentPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_COLOR_POST_TRANSPARENCY
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	bento::RectangleTexture& RenderTarget::PositionTexture()
	{
		return m_positionTexture;
	}

	bento::RectangleTexture& RenderTarget::ColorTexture()
	{
		return m_colorTexture;
	}

	bento::RectangleTexture& RenderTarget::ColorPostTransparencyTexture()
	{
		return m_colorPostTransparencyTexture;
	}
}