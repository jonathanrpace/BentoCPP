#pragma once

// std
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

// bento
#include <bento/core/SharedObject.h>
#include <bento/core/IRenderer.h>
#include <bento/core/IRenderPass.h>
#include <bento/core/RenderPhase.h>
#include <bento/renderTargets/DeferredRenderTarget.h>
#include <bento/shaders/RectTextureToScreenShader.h>

namespace bento
{
	class DefaultRenderer : 
		public IRenderer,
		public bento::SharedObject<DefaultRenderer>
	{
	public:
		DefaultRenderer();
		~DefaultRenderer();

		// IRenderer
		virtual void BindToScene(bento::Scene * const _scene) override;
		virtual void UnbindFromScene(bento::Scene * const _scene) override;
		virtual void Update(double dt) override;
		virtual void AddRenderPass(RenderPassPtr) override;
		virtual void RemoveRenderPass(RenderPassPtr) override;

		// ISceneObejct
		virtual const std::type_info & typeInfo() override;

		bento::Entity GetCamera() { return m_camera;  }

	private:
		// Types
		typedef std::vector<RenderPassPtr>				RenderPassList;
		typedef std::map<RenderPhase, RenderPassList*>	RenderPassByPhaseMap;

		// Methods
		void AddRenderPhase(RenderPhase _renderPhase);
		void RenderPassesInPhase(RenderPhase _renderPhase);

		// Member variables
		bento::Scene * m_scene;
		RenderPassByPhaseMap m_renderPassesByPhase;
		bento::Entity m_camera;
		DeferredRenderTarget m_deferredRenderTarget;
		RectTextureToScreenShader m_rectTextureToScreenShader;
	};
}