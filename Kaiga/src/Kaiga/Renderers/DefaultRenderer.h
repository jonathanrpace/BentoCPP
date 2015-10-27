#pragma once

// std
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

// ramen
#include <Ramen/Core/SharedObject.h>

// kaiga
#include <Kaiga/Core/IRenderer.h>
#include <Kaiga/Core/IRenderPass.h>
#include <Kaiga/Core/RenderPhase.h>
#include <Kaiga/RenderTargets/DeferredRenderTarget.h>
#include <Kaiga/Shaders/RectTextureToScreenShader.h>

namespace Kaiga
{
	class DefaultRenderer : 
		public IRenderer,
		public Ramen::SharedObject<DefaultRenderer>
	{
	public:
		DefaultRenderer();
		~DefaultRenderer();

		// IRenderer
		virtual void BindToScene(Ramen::Scene * const _scene) override;
		virtual void UnbindFromScene(Ramen::Scene * const _scene) override;
		virtual void Update(double dt) override;
		virtual void AddRenderPass(RenderPassPtr) override;
		virtual void RemoveRenderPass(RenderPassPtr) override;

		// ISceneObejct
		virtual const std::type_info & typeInfo() override;

		Ramen::Entity GetCamera() { return m_camera;  }

	private:
		// Types
		typedef std::vector<RenderPassPtr>				RenderPassList;
		typedef std::map<RenderPhase, RenderPassList*>	RenderPassByPhaseMap;

		// Methods
		void AddRenderPhase(RenderPhase _renderPhase);
		void RenderPassesInPhase(RenderPhase _renderPhase);

		// Member variables
		Ramen::Scene * m_scene;
		RenderPassByPhaseMap m_renderPassesByPhase;
		Ramen::Entity m_camera;
		DeferredRenderTarget m_deferredRenderTarget;
		RectTextureToScreenShader m_rectTextureToScreenShader;
	};
}