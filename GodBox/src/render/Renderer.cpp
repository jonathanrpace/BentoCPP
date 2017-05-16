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

// nv
//#include <nvToolsExt.h>


namespace godBox
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	Renderer::Renderer(std::string _name)
		: RendererBase(_name, typeid(RendererBase))
		, m_blurredRenderTargetA(256, 256, true, false)
		, m_blurredRenderTargetB(256, 256, true, false)
	{
		AddRenderPhase(eRenderPhase_Forward);
		AddRenderPhase(eRenderPhase_Transparent);
		AddRenderPhase(eRenderPhase_UI);

		m_blurredRenderTargetA.AttachTexture(GL_COLOR_ATTACHMENT0, m_renderTarget.BlurredColorTextureA());
		m_blurredRenderTargetB.AttachTexture(GL_COLOR_ATTACHMENT0, m_renderTarget.BlurredColorTextureB());
	}

	Renderer::~Renderer()
	{
		bento::RenderParams::SetCameraTransform(nullptr);
	}

	void Renderer::Advance(double dt)
	{
		//nvtxRangePushA(__FUNCTION__);

		// Update RenderParams
		//nvtxRangePushA("Setup Render Params");
		auto lens = m_scene->GetComponentForEntity<PerspectiveLens>(m_camera);
		IWindow& window = m_scene->GetWindow();
		ivec2 windowSize = window.GetWindowSize();
		lens->SetAspectRatio((float)windowSize.x / windowSize.y);
		bento::RenderParams::SetBackBufferDimensions(windowSize.x, windowSize.y);
		auto cameraTransform = m_scene->GetComponentForEntity<Transform>(m_camera);
		bento::RenderParams::SetViewMatrices(cameraTransform->matrix, *lens);
		godBox::RenderParams::SetRenderTarget(m_renderTarget);
		m_renderTarget.SetSize(windowSize.x, windowSize.y);
		//nvtxRangePop();

		// Setup some default render states
		glViewport(0, 0, windowSize.x, windowSize.y);
		m_renderTarget.Clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);
		
		// Forward Phase
		//nvtxRangePushA("PHASE: Forward");
		m_renderTarget.BindForForwardPhase();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		RenderPassesInPhase(eRenderPhase_Forward, dt);
		//nvtxRangePop();

		////////////////////////////////////////////////////////////
		// Kawase blur
		// https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
		////////////////////////////////////////////////////////////
		
		static const GLenum blurDrawBuffers[] = {
			GL_COLOR_ATTACHMENT0
		};

		// Do all our blurring at 1/4 size
		//nvtxRangePushA("Downsample to 1/4 res");
		m_blurredRenderTargetA.SetSize(m_renderTarget.Width() >> 1, m_renderTarget.Height() >> 1);
		m_blurredRenderTargetB.SetSize(m_renderTarget.Width() >> 1, m_renderTarget.Height() >> 1);
		glViewport(0, 0, m_blurredRenderTargetA.Width(), m_blurredRenderTargetA.Height());

		// Draw our color buffer to blurredBufferA
		m_blurredRenderTargetA.SetDrawBuffers(blurDrawBuffers, 1);
		m_rectTextureToScreenShader.Render(m_renderTarget.ColorTexture(), 0.5f);
		//nvtxRangePop();
		
		//nvtxRangePushA("Kawase blur");
		bool switcher = false;
		float offset = 0.5f;
		for ( int i = 0; i < 7; i++ )
		{
			if ( switcher )
				m_blurredRenderTargetA.SetDrawBuffers(blurDrawBuffers, 1);
			else
				m_blurredRenderTargetB.SetDrawBuffers(blurDrawBuffers, 1);

			m_blurShader.Render( switcher ? m_renderTarget.BlurredColorTextureB() : m_renderTarget.BlurredColorTextureA(), offset );

			switcher = !switcher;
			offset += 1.0f;
		}
		//nvtxRangePop();
		
		////////////////////////////////////////////////////////////


		//nvtxRangePushA("Copy to post transparent");
		glViewport(0, 0, windowSize.x, windowSize.y);
		// Copy the current forward buffer over to the 'post transparent' buffer.
		// If we dont do this, only fragments touched by a transparent shader will appear in the final output.
		m_renderTarget.BindForTransparentPhase();
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		m_rectTextureToScreenShader.Render(m_renderTarget.ColorTexture());
		//nvtxRangePop();

		// Transparent Phase
		//nvtxRangePushA("PHASE: Transparent");
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		RenderPassesInPhase(eRenderPhase_Transparent, dt);
		//nvtxRangePop();

		// Switch draw target to back buffer
		//nvtxRangePushA("PHASE: Copy to backbuffer");
		glViewport(0, 0, windowSize.x, windowSize.y);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
		glEnable( GL_FRAMEBUFFER_SRGB );

		m_resolveShader.Render(
			m_renderTarget.ColorPostTransparencyTexture(),
			switcher ? m_renderTarget.BlurredColorTextureA() : m_renderTarget.BlurredColorTextureB()
		);

		//m_rectTextureToScreenShader.Render(m_renderTarget.ColorPostTransparencyTexture());
		//m_rectTextureToScreenShader.Render(switcher ? m_renderTarget.BlurredColorTextureA() : m_renderTarget.BlurredColorTextureB());
		glDisable(GL_FRAMEBUFFER_SRGB);
		//nvtxRangePop();

		// UI Phase
		//nvtxRangePushA("PHASE: UI");
		RenderPassesInPhase(eRenderPhase_UI, dt);
		//nvtxRangePop();

		//nvtxRangePop();
	}
}