#include "RendererBase.h"

#include <gl/glew.h>
#include <assert.h>
#include <utility>

#include <bento/components/PerspectiveLens.h>
#include <bento/components/Transform.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	RendererBase::RendererBase(std::string _name, const std::type_info& _typeInfo)
		: Process(_name, _typeInfo)
		, m_scene(NULL)
	{
		
	}

	RendererBase::~RendererBase()
	{
		for (auto iter : m_renderPassesByPhase)
		{
			RenderPassList* list = iter.second;
			if (m_scene != NULL)
			{
				for (RenderPassPtr renderPass : *list)
				{
					renderPass->UnbindFromScene(*m_scene);
				}
			}
			list->clear();
			delete list;
		}
		m_renderPassesByPhase.clear();
		m_scene = NULL;
	}

	void RendererBase::BindToScene(bento::Scene& _scene)
	{
		assert(m_scene == NULL);

		m_scene = &_scene;

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
				renderPass->BindToScene(_scene);
			}
		}
	}

	void RendererBase::UnbindFromScene(bento::Scene& _scene)
	{
		assert(m_scene != NULL);

		m_scene->RemoveEntity(m_camera);
		m_camera.reset();

		for (auto iter : m_renderPassesByPhase)
		{
			RenderPassList* list = iter.second;
			for (RenderPassPtr renderPass : *list)
			{
				renderPass->BindToScene(_scene);
			}
		}
		m_scene = NULL;
	}

	void RendererBase::AddRenderPass(RenderPassPtr _renderPass)
	{
		auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
		assert(passes);
		passes->push_back(_renderPass);

		if (m_scene)
			_renderPass->BindToScene(*m_scene);
	}

	void RendererBase::RemoveRenderPass(RenderPassPtr _renderPass)
	{
		auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
		passes->erase(std::find(passes->begin(), passes->end(), _renderPass));

		if (m_scene)
			_renderPass->UnbindFromScene(*m_scene);
	}

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE
	//////////////////////////////////////////////////////////////////////////

	void RendererBase::AddRenderPhase(eRenderPhase _renderPhase)
	{
		// TODO - assert phase does not already exist
		RenderPassList* list = new RenderPassList();
		m_renderPassesByPhase.insert(std::make_pair(_renderPhase, list));
	}

	void RendererBase::RenderPassesInPhase(eRenderPhase _renderPhase, double _dt)
	{
		RenderPassList passes = *m_renderPassesByPhase[_renderPhase];
		for (RenderPassPtr renderPass : passes)
		{
			renderPass->Advance(_dt);
		}
	}
}