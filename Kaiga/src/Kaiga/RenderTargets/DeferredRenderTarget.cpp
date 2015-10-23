#include "DeferredRenderTarget.h"
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

	const std::vector<GLenum> DeferredRenderTarget::ALL_DRAW_BUFFERS = {
		DRAW_BUFFER_POSITION,
		DRAW_BUFFER_NORMAL,
		DRAW_BUFFER_ALBEDO,
		DRAW_BUFFER_MATERIAL,
		DRAW_BUFFER_DIRECT_LIGHT,
		DRAW_BUFFER_INDIRECT_LIGHT,
		DRAW_BUFFER_OUTPUT
	};
	const std::vector<GLenum> DeferredRenderTarget::G_PHASE_DRAW_BUFFERS = {
		DRAW_BUFFER_POSITION,
		DRAW_BUFFER_NORMAL,
		DRAW_BUFFER_ALBEDO,
		DRAW_BUFFER_MATERIAL,
		DRAW_BUFFER_DIRECT_LIGHT		// TODO - NEED THIS?
	};
	const std::vector<GLenum> DeferredRenderTarget::DIRECT_LIGHT_PHASE_DRAW_BUFFERS = {
		DRAW_BUFFER_DIRECT_LIGHT
	};
	const std::vector<GLenum> DeferredRenderTarget::INDIRECT_LIGHT_PHASE_DRAW_BUFFERS = {
		DRAW_BUFFER_INDIRECT_LIGHT
	};
	const std::vector<GLenum> DeferredRenderTarget::RESOLVE_PHASE_DRAW_BUFFERS = {
		DRAW_BUFFER_OUTPUT
	};

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

	void DeferredRenderTarget::Clear()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(ALL_DRAW_BUFFERS.size(), &ALL_DRAW_BUFFERS[0]));
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClearDepth(1.0f));
		GL_CHECK(glClearStencil(0));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
	}

	void DeferredRenderTarget::BindForGPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(G_PHASE_DRAW_BUFFERS.size(), &G_PHASE_DRAW_BUFFERS[0]));
	}

	void DeferredRenderTarget::BindForDirectLightPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(DIRECT_LIGHT_PHASE_DRAW_BUFFERS.size(), &DIRECT_LIGHT_PHASE_DRAW_BUFFERS[0]));
	}

	void DeferredRenderTarget::BindForIndirectLightPhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(INDIRECT_LIGHT_PHASE_DRAW_BUFFERS.size(), &INDIRECT_LIGHT_PHASE_DRAW_BUFFERS[0]));
	}

	void DeferredRenderTarget::BindForResolvePhase()
	{
		ValidateNow();
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frameBuffer));
		GL_CHECK(glDrawBuffers(RESOLVE_PHASE_DRAW_BUFFERS.size(), &RESOLVE_PHASE_DRAW_BUFFERS[0]));
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