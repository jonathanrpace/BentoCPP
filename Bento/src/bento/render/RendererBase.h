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

namespace bento
{
	class RendererBase 
		: public Process
		, public SharedObject<RendererBase>
	{
	public:
		RendererBase(std::string _name, const std::type_info& _typeInfo);
		virtual ~RendererBase();

		// From Process
		virtual void BindToScene(bento::Scene& _scene) override;
		virtual void UnbindFromScene(bento::Scene& _scene) override;

		void AddRenderPass(RenderPassPtr);
		void RemoveRenderPass(RenderPassPtr);

		EntityPtr GetCamera() { return m_camera; }

	protected:
		Scene * m_scene;
		EntityPtr m_camera;

		// Methods
		void AddRenderPhase(eRenderPhase _renderPhase);
		void RenderPassesInPhase(eRenderPhase _renderPhase, double dt);

	private:
		// Types
		typedef std::vector<RenderPassPtr>				RenderPassList;
		typedef std::map<eRenderPhase, RenderPassList*>	RenderPassByPhaseMap;

		// Member variables
		RenderPassByPhaseMap m_renderPassesByPhase;
	};
}