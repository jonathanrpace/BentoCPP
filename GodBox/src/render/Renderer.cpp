#include "Renderer.h"

#include <gl/glew.h>
#include <assert.h>
#include <utility>

// bento
#include <bento/components/PerspectiveLens.h>
#include <bento/components/Transform.h>
#include <bento/render/RenderParams.h>

// app
#include <render/RenderParams.h>
#include <render/eRenderPhase.h>

namespace godBox
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	Renderer::Renderer(std::string _name)
		: RendererBase(_name, typeid(RendererBase))
		, m_blurredRenderTarget(256, 256, true, false)
		, m_blurredRenderTarget2(256, 256, true, false)
		, m_blurredRenderTarget3(256, 256, true, false)
	{
		AddRenderPhase(eRenderPhase_Forward);
		AddRenderPhase(eRenderPhase_Transparent);
		AddRenderPhase(eRenderPhase_UI);

		m_blurredRenderTarget.AttachTexture(GL_COLOR_ATTACHMENT0, m_blurredBuffer);
		m_blurredRenderTarget2.AttachTexture(GL_COLOR_ATTACHMENT0, m_blurredBuffer2);
		m_blurredRenderTarget3.AttachTexture(GL_COLOR_ATTACHMENT0, m_blurredBuffer3);
	}

	Renderer::~Renderer()
	{
		bento::RenderParams::SetCameraTransform(nullptr);
	}

	void Renderer::Advance(double dt)
	{
		// Update RenderParams
		auto lens = m_scene->GetComponentForEntity<PerspectiveLens>(m_camera);
		IWindow& window = m_scene->GetWindow();
		ivec2 windowSize = window.GetWindowSize();
		lens->SetAspectRatio((float)windowSize.x / windowSize.y);
		bento::RenderParams::SetBackBufferDimensions(windowSize.x, windowSize.y);
		auto cameraTransform = m_scene->GetComponentForEntity<Transform>(m_camera);
		bento::RenderParams::SetViewMatrices(cameraTransform->matrix, *lens);
		godBox::RenderParams::SetRenderTarget(m_renderTarget);
		m_renderTarget.SetSize(windowSize.x, windowSize.y);
		
		// Setup some default render states
		glViewport(0, 0, windowSize.x, windowSize.y);
		m_renderTarget.Clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		
		// Forward Phase
		m_renderTarget.BindForForwardPhase();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		RenderPassesInPhase(eRenderPhase_Forward, dt);



		////////////////////////////////////
		m_blurredRenderTarget.SetSize(m_renderTarget.Width() >> 1, m_renderTarget.Height() >> 1);
		static const GLenum blurDrawBuffers[] = {
			GL_COLOR_ATTACHMENT0
		};
		m_blurredRenderTarget.SetDrawBuffers(blurDrawBuffers, 1);
		glViewport(0, 0, m_blurredRenderTarget.Width(), m_blurredRenderTarget.Height());
		m_blurShader.Render(m_renderTarget.ColorTexture());

		m_blurredRenderTarget2.SetSize(m_renderTarget.Width() >> 2, m_renderTarget.Height() >> 2);
		m_blurredRenderTarget2.SetDrawBuffers(blurDrawBuffers, 1);
		glViewport(0, 0, m_blurredRenderTarget2.Width(), m_blurredRenderTarget2.Height());
		m_blurShader.Render(m_blurredBuffer);

		m_blurredRenderTarget3.SetSize(m_renderTarget.Width() >> 3, m_renderTarget.Height() >> 3);
		m_blurredRenderTarget3.SetDrawBuffers(blurDrawBuffers, 1);
		glViewport(0, 0, m_blurredRenderTarget3.Width(), m_blurredRenderTarget3.Height());
		m_blurShader.Render(m_blurredBuffer2);

		////////////////////////////////////


		glViewport(0, 0, windowSize.x, windowSize.y);

		// Copy the current forward buffer over to the 'post transparent' buffer.
		// If we dont do this, only fragments touched by a transparent shader will appear in the final output.
		m_renderTarget.BindForTransparentPhase();
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		m_rectTextureToScreenShader.Render(m_renderTarget.ColorTexture());

		// Transparent Phase
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		RenderPassesInPhase(eRenderPhase_Transparent, dt);

		// Switch draw target to back buffer
		glViewport(0, 0, windowSize.x, windowSize.y);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
		glEnable( GL_FRAMEBUFFER_SRGB );
		//m_rectTextureToScreenShader.Render(m_renderTarget.ColorPostTransparencyTexture());
		m_rectTextureToScreenShader.Render(m_blurredBuffer3);
		glDisable(GL_FRAMEBUFFER_SRGB);

		// UI Phase
		RenderPassesInPhase(eRenderPhase_UI, dt);
	}
}