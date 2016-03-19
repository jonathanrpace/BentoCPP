#include "DefaultRenderer.h"

#include <gl/glew.h>
#include <assert.h>
#include <utility>

#include <bento/render/RenderParams.h>
#include <bento/components/PerspectiveLens.h>
#include <bento/components/Transform.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	DefaultRenderer::DefaultRenderer(std::string _name)
		: Process(_name, typeid(DefaultRenderer))
		, m_scene(NULL)
	{
		AddRenderPhase(eRenderPhase_OffScreen);
		AddRenderPhase(eRenderPhase_G);
		AddRenderPhase(eRenderPhase_Forward);
		AddRenderPhase(eRenderPhase_DirectLight);
		AddRenderPhase(eRenderPhase_IndirectLight);
		AddRenderPhase(eRenderPhase_PostLight);
		AddRenderPhase(eRenderPhase_UI);
	}

	DefaultRenderer::~DefaultRenderer()
	{
		for (auto iter : m_renderPassesByPhase)
		{
			RenderPassList* list = iter.second;
			if (m_scene != NULL)
			{
				for (RenderPassPtr renderPass : *list)
				{
					renderPass->UnbindFromScene(m_scene);
				}
			}
			list->clear();
			delete list;
		}
		m_renderPassesByPhase.clear();
		m_scene = NULL;
	}

	void DefaultRenderer::BindToScene(bento::Scene * const _scene)
	{
		assert(m_scene == NULL);

		m_scene = _scene;

		m_camera = Entity::Create();
		m_camera->Name("Camera");
		m_scene->AddEntity(m_camera);
		
		auto lens = PerspectiveLens::Create();
		auto transform = Transform::Create();
		m_scene->AddComponentToEntity(lens, m_camera);
		m_scene->AddComponentToEntity(transform, m_camera);

		for (auto iter : m_renderPassesByPhase)
		{
			RenderPassList* list = iter.second;
			for (RenderPassPtr renderPass : *list)
			{
				renderPass->BindToScene(m_scene);
			}
		}
	}

	void DefaultRenderer::UnbindFromScene(bento::Scene * const _scene)
	{
		assert(m_scene != NULL);

		m_scene->RemoveEntity(m_camera);
		m_camera.reset();

		for (auto iter : m_renderPassesByPhase)
		{
			RenderPassList* list = iter.second;
			for (RenderPassPtr renderPass : *list)
			{
				renderPass->BindToScene(m_scene);
			}
		}
		m_scene = NULL;
	}

	void DefaultRenderer::Advance(double dt)
	{
		// Update RenderParams
		auto lens = m_scene->GetComponentForEntity<PerspectiveLens>(m_camera);
		auto window = m_scene->GetWindow();
		ivec2 windowSize = window->GetWindowSize();
		lens->SetAspectRatio((float)windowSize.x / windowSize.y);
		RenderParams::SetBackBufferDimensions(windowSize.x, windowSize.y);
		auto cameraTransform = m_scene->GetComponentForEntity<Transform>(m_camera);
		RenderParams::SetViewMatrices(cameraTransform->matrix, *lens);
		RenderParams::DeferedRenderTarget = &m_deferredRenderTarget;
		m_deferredRenderTarget.SetSize(windowSize.x, windowSize.y);
		
		// Setup some default render states
		glViewport(0, 0, windowSize.x, windowSize.y);
		m_deferredRenderTarget.Clear();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);

		// Off Screen pass
		RenderPassesInPhase(eRenderPhase_OffScreen, dt);

		glViewport(0, 0, windowSize.x, windowSize.y);

		// G-Pass
		m_deferredRenderTarget.BindForGPhase();
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		RenderPassesInPhase(eRenderPhase_G, dt);

		// Forward pass

		// First copy outputA to outputB
		m_deferredRenderTarget.BindForForwardPhase();
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		m_rectTextureToScreenShader.Render(m_deferredRenderTarget.OutputTextureA());

		// Render forward passes
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		RenderPassesInPhase(eRenderPhase_Forward, dt);

		// Switch draw target to back buffer
		glViewport(0, 0, windowSize.x, windowSize.y);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
		//GL.Enable( EnableCap.FramebufferSrgb );
		m_rectTextureToScreenShader.Render(m_deferredRenderTarget.OutputTextureB());

		// UI-Pass
		RenderPassesInPhase(eRenderPhase_UI, dt);
	}

	void DefaultRenderer::AddRenderPass(RenderPassPtr _renderPass)
	{
		auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
		assert(passes);
		passes->push_back(_renderPass);

		if (m_scene)
			_renderPass->BindToScene(m_scene);
	}

	void DefaultRenderer::RemoveRenderPass(RenderPassPtr _renderPass)
	{
		auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
		passes->erase(std::find(passes->begin(), passes->end(), _renderPass));

		if (m_scene)
			_renderPass->UnbindFromScene(m_scene);
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void DefaultRenderer::AddRenderPhase(RenderPhase _renderPhase)
	{
		// TODO - assert phase does not already exist
		RenderPassList* list = new RenderPassList();
		m_renderPassesByPhase.insert(std::make_pair(_renderPhase, list));
	}

	void DefaultRenderer::RenderPassesInPhase(RenderPhase _renderPhase, double _dt)
	{
		RenderPassList passes = *m_renderPassesByPhase[_renderPhase];
		for (RenderPassPtr renderPass : passes)
		{
			renderPass->Advance(_dt);
		}
	}
}