#include "DeferredRenderTarget.h"

#include <Kaiga/Util/GLErrorUtil.h>

namespace Kaiga
{
	
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_POSITION = GL_COLOR_ATTACHMENT0;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_NORMAL = GL_COLOR_ATTACHMENT1;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_ALBEDO = GL_COLOR_ATTACHMENT2;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_MATERIAL = GL_COLOR_ATTACHMENT3;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT = GL_COLOR_ATTACHMENT4;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_INDIRECT_LIGHT = GL_COLOR_ATTACHMENT5;
	const GLuint DeferredRenderTarget::FRAME_BUFFER_ATTACHMENT_OUTPUT = GL_COLOR_ATTACHMENT6;

	const GLuint DeferredRenderTarget::DRAW_BUFFER_POSITION = GL_DRAW_BUFFER0;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_NORMAL = GL_DRAW_BUFFER1;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_ALBEDO = GL_DRAW_BUFFER2;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_MATERIAL = GL_DRAW_BUFFER3;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_DIRECT_LIGHT = GL_DRAW_BUFFER4;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_INDIRECT_LIGHT = GL_DRAW_BUFFER5;
	const GLuint DeferredRenderTarget::DRAW_BUFFER_OUTPUT = GL_DRAW_BUFFER6;
	
	DeferredRenderTarget::DeferredRenderTarget
	(
		int _width, 
		int _height
	)
		: RenderTargetBase(_width, _height, true)
		, m_normalTexture(_width, _height, GL_RGBA16F, GL_NEAREST, GL_NEAREST)
		, m_positionTexture(_width, _height, GL_RGBA16F, GL_NEAREST, GL_NEAREST)
		, m_albedoTexture(_width, _height, GL_RGBA16F)
		, m_materialTexture(_width, _height, GL_RGBA16F)
		, m_directLightTexture(_width, _height, GL_RGBA16F)
		, m_indirectLightTexture(_width, _height, GL_RGBA16F)
		, m_outputTexture(_width, _height, GL_RGBA16F)
	{
		AttachTexture(FRAME_BUFFER_ATTACHMENT_POSITION, &m_positionTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_NORMAL, &m_normalTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_ALBEDO, &m_albedoTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_MATERIAL, &m_materialTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT, &m_directLightTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_INDIRECT_LIGHT, &m_indirectLightTexture);
		AttachTexture(FRAME_BUFFER_ATTACHMENT_OUTPUT, &m_outputTexture);
	}

	DeferredRenderTarget::~DeferredRenderTarget()
	{
		
	}

	void DeferredRenderTarget::Clear()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_POSITION,
			FRAME_BUFFER_ATTACHMENT_NORMAL,
			FRAME_BUFFER_ATTACHMENT_ALBEDO,
			FRAME_BUFFER_ATTACHMENT_MATERIAL,
			FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT,
			FRAME_BUFFER_ATTACHMENT_INDIRECT_LIGHT,
			FRAME_BUFFER_ATTACHMENT_OUTPUT
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));

		GL_CHECK(glClearColor(0.0f, 1.0f, 0.0f, 0.0f));
		GL_CHECK(glClearDepth(1.0f));
		GL_CHECK(glClearStencil(0));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	}

	void DeferredRenderTarget::BindForGPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_POSITION,
			FRAME_BUFFER_ATTACHMENT_NORMAL,
			FRAME_BUFFER_ATTACHMENT_ALBEDO,
			FRAME_BUFFER_ATTACHMENT_MATERIAL,
			FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT // TODO - NEED THIS?
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	void DeferredRenderTarget::BindForDirectLightPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	void DeferredRenderTarget::BindForIndirectLightPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_INDIRECT_LIGHT
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	void DeferredRenderTarget::BindForResolvePhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));

		static const GLenum drawBuffers[] = {
			FRAME_BUFFER_ATTACHMENT_OUTPUT
		};
		GL_CHECK(glDrawBuffers(sizeof(drawBuffers) / sizeof(drawBuffers[0]), drawBuffers));
	}

	void DeferredRenderTarget::BindForNoDraw()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		glDrawBuffer(GL_NONE);
	}
	
	RectangleTexture* DeferredRenderTarget::NormalTexture()
	{
		return &m_normalTexture;
	}

	RectangleTexture* DeferredRenderTarget::PositionTexture()
	{
		return &m_positionTexture;
	}

	RectangleTexture* DeferredRenderTarget::AlbedoTexture()
	{
		return &m_albedoTexture;
	}

	RectangleTexture* DeferredRenderTarget::MaterialTexture()
	{
		return &m_materialTexture;
	}

	RectangleTexture* DeferredRenderTarget::DirectLightTexture()
	{
		return &m_directLightTexture;
	}

	RectangleTexture* DeferredRenderTarget::IndirectLightTexture()
	{
		return &m_indirectLightTexture;
	}

	RectangleTexture* DeferredRenderTarget::OutputTexture()
	{
		return &m_outputTexture;
	}
}