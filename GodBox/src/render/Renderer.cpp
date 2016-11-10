#include "Renderer.h"

#include <gl/glew.h>
#include <assert.h>
#include <utility>

// bento
#include <bento/components/PerspectiveLens.h>
#include <bento/components/Transform.h>

// app
#include <render/RenderParams.h>

namespace godBox
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	Renderer::Renderer(std::string _name)
		: RendererBase(_name, typeid(Renderer))
	{
		AddRenderPhase(eRenderPhase_OffScreen);
		AddRenderPhase(eRenderPhase_Transparent);
		AddRenderPhase(eRenderPhase_UI);
	}

	Renderer::~Renderer()
	{
		RendererBase::~RendererBase();
	}

	void Renderer::Advance(double dt)
	{
		// Update RenderParams
		auto lens = m_scene->GetComponentForEntity<bento::PerspectiveLens>(m_camera);
		bento::IWindow& window = m_scene->GetWindow();
		ivec2 windowSize = window.GetWindowSize();
		lens->SetAspectRatio((float)windowSize.x / windowSize.y);
		bento::RenderParams::SetBackBufferDimensions(windowSize.x, windowSize.y);
		auto cameraTransform = m_scene->GetComponentForEntity<bento::Transform>(m_camera);
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

		// Pre-transparent phase
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
		m_rectTextureToScreenShader.Render(m_renderTarget.ColorPostTransparencyTexture());
		glDisable(GL_FRAMEBUFFER_SRGB);

		// UI Phase
		RenderPassesInPhase(eRenderPhase_UI, dt);
	}
}