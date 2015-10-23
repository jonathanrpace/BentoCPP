#include "DefaultRenderer.h"

#include <glew.h>
#include <assert.h>
#include <utility>

#include <Kaiga/Core/RenderParams.h>
#include <Kaiga/Components/PerspectiveLens.h>
#include <Kaiga/Components/Transform.h>

//////////////////////////////////////////////////////////////////////////
// PUBLIC
//////////////////////////////////////////////////////////////////////////
Kaiga::DefaultRenderer::DefaultRenderer() 
	: m_scene(NULL)
	, m_renderPassesByPhase()
	, m_deferredRenderTarget()
{
	AddRenderPhase(eRenderPhase_G);
	AddRenderPhase(eRenderPhase_DirectLight);
	AddRenderPhase(eRenderPhase_IndirectLight);
	AddRenderPhase(eRenderPhase_PostLight);
}

Kaiga::DefaultRenderer::~DefaultRenderer()
{
	for (auto iter : m_renderPassesByPhase)	{
		RenderPassList* list = iter.second;
		if (m_scene != NULL) {
			for (RenderPassPtr renderPass : *list)	{
				renderPass->UnbindFromScene(*m_scene);
			}
		}
		list->clear();
		delete list;
	}
	m_renderPassesByPhase.clear();
	m_scene = NULL;
}

const std::type_info & Kaiga::DefaultRenderer::typeInfo()
{
	return typeid(Kaiga::DefaultRenderer);
}

void Kaiga::DefaultRenderer::BindToScene(Ramen::Scene * const _scene)
{
	assert(m_scene == NULL);

	m_scene = _scene;

	m_camera = m_scene->CreateEntity();
	auto lens = PerspectiveLens::Create();
	auto transform = Transform::Create();
	m_scene->AddComponentToEntity(lens, m_camera);
	m_scene->AddComponentToEntity(transform, m_camera);

	for (auto iter : m_renderPassesByPhase)	{
		RenderPassList* list = iter.second;
		for (RenderPassPtr renderPass : *list) {
			renderPass->BindToScene(*m_scene);
		}
	}
}

void Kaiga::DefaultRenderer::UnbindFromScene(Ramen::Scene * const _scene)
{
	assert(m_scene != NULL);

	m_scene->DestroyEntity(m_camera);

	for (auto iter : m_renderPassesByPhase)	{
		RenderPassList* list = iter.second;
		for (RenderPassPtr renderPass : *list) {
			renderPass->BindToScene(*m_scene);
		}
	}
	m_scene = NULL;
}

void Kaiga::DefaultRenderer::Update(double dt)
{
	auto lens = m_scene->GetComponentForEntity<PerspectiveLens>(m_camera);
	lens->SetAspectRatio(m_scene->GetWindow()->GetWindowSize().x / m_scene->GetWindow()->GetWindowSize().y);

	auto cameraTransform = m_scene->GetComponentForEntity<Transform>(m_camera);

	Kaiga::RenderParams::SetViewMatrices(cameraTransform->matrix, *lens);

	Kaiga::RenderParams::DeferedRenderTarget = &m_deferredRenderTarget;

	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderPassesInPhase(eRenderPhase_G);
}

void Kaiga::DefaultRenderer::AddRenderPass(RenderPassPtr _renderPass)
{
	auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
	passes->push_back(_renderPass);

	if (m_scene != NULL)
		_renderPass->BindToScene(*m_scene);
}

void Kaiga::DefaultRenderer::RemoveRenderPass(RenderPassPtr _renderPass)
{
	auto passes = m_renderPassesByPhase[_renderPass->GetRenderPhase()];
	passes->erase(std::find(passes->begin(), passes->end(), _renderPass));

	if (m_scene != NULL)
		_renderPass->UnbindFromScene(*m_scene);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////

void Kaiga::DefaultRenderer::AddRenderPhase(RenderPhase _renderPhase)
{
	// TODO - assert phase does not already exist
	RenderPassList* list = new RenderPassList();
	m_renderPassesByPhase.insert(std::make_pair(_renderPhase, list));
}

void Kaiga::DefaultRenderer::RenderPassesInPhase(RenderPhase _renderPhase)
{
	RenderPassList passes = *m_renderPassesByPhase[_renderPhase];
	for(RenderPassPtr renderPass : passes)	{
		renderPass->Render();
	}
}
