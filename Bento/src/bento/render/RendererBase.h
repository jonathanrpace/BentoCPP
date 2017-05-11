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
#include <bento/core/Scene.h>

namespace bento
{
	class RendererBase 
		: public Process
	{
	public:
		RendererBase(std::string _name, const std::type_info& _typeInfo);
		virtual ~RendererBase();

		// Types
		typedef std::vector<RenderPassPtr>				RenderPassList;
		typedef std::map<int, RenderPassList*>			RenderPassByPhaseMap;

		// From Process
		virtual void BindToScene(bento::Scene& _scene) override;
		virtual void UnbindFromScene(bento::Scene& _scene) override;
		virtual void Advance(double dt) = 0;

		void AddRenderPass(RenderPassPtr);
		void RemoveRenderPass(RenderPassPtr);
		RenderPassList RenderPasses() { return m_renderPasses; }

		EntityPtr GetCamera() { return m_camera; }

	protected:
		Scene * m_scene;
		EntityPtr m_camera;

		// Methods
		void AddRenderPhase(int _renderPhase);
		void RenderPassesInPhase(int _renderPhase, double dt);

	private:
		
		// Member variables
		RenderPassByPhaseMap m_renderPassesByPhase;
		RenderPassList m_renderPasses;
	};
}