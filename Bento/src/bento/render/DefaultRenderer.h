#pragma once

// std
#include <map>
#include <string>

// bento
#include <bento.h>
#include <bento/core/SharedObject.h>
#include <bento/core/SceneObject.h>
#include <bento/core/Process.h>
#include <bento/core/RenderPass.h>
#include <bento/core/eRenderPhase.h>
#include <bento/core/Scene.h>
#include <bento/render/DeferredRenderTarget.h>
#include <bento/render/shaders/RectTextureToScreenShader.h>

namespace bento
{
	class DefaultRenderer 
		: public Process
		, public SharedObject<DefaultRenderer>
	{
	public:
		DefaultRenderer(std::string _name = "DefaultRenderer");
		~DefaultRenderer();

		// From Process
		virtual void BindToScene(bento::Scene& _scene) override;
		virtual void UnbindFromScene(bento::Scene& _scene) override;
		virtual void Advance(double dt) override;

		void AddRenderPass(RenderPassPtr);
		void RemoveRenderPass(RenderPassPtr);

		EntityPtr GetCamera() { return m_camera; }

	private:
		// Types
		typedef std::vector<RenderPassPtr>				RenderPassList;
		typedef std::map<eRenderPhase, RenderPassList*>	RenderPassByPhaseMap;

		// Methods
		void AddRenderPhase(eRenderPhase _renderPhase);
		void RenderPassesInPhase(eRenderPhase _renderPhase, double dt);

		// Member variables
		Scene * m_scene;
		RenderPassByPhaseMap m_renderPassesByPhase;
		EntityPtr m_camera;
		DeferredRenderTarget m_deferredRenderTarget;
		RectTextureToScreenShader m_rectTextureToScreenShader;
	};
}