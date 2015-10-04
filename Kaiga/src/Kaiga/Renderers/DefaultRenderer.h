#pragma once

// std
#include <vector>
#include <map>
#include <memory>

// Ramen
#include <Ramen\Core\Scene.h>

// Kaiga
#include <Kaiga\Core\IRenderer.h>
#include <Kaiga\Core\RenderPhase.h>

namespace Kaiga
{
	class DefaultRenderer : public IRenderer
	{
	public:

		static std::shared_ptr<DefaultRenderer> Create()
		{
			return std::make_shared<DefaultRenderer>();
		}

		DefaultRenderer();
		~DefaultRenderer();

		// IRenderer
		virtual void BindToScene(Ramen::Scene * const _scene) override;
		virtual void UnbindFromScene(Ramen::Scene * const _scene) override;
		virtual void Update(double dt) override;
		virtual void AddRenderPass(RenderPassPtr) override;
		virtual void RemoveRenderPass(RenderPassPtr) override;

	private:
		// Types
		typedef std::vector<RenderPassPtr> RenderPassList;
		typedef std::map<RenderPhase, RenderPassList*> RenderPassByPhaseMap;

		// Methods
		void AddRenderPhase(RenderPhase _renderPhase);
		void RenderPassesInPhase(RenderPhase _renderPhase);

		// Member variables
		Ramen::Scene * m_scene;
		RenderPassByPhaseMap m_renderPassesByPhase;
		Ramen::Entity m_camera;
	};
}