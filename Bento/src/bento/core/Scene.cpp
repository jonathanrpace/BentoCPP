#include "Scene.h"

namespace bento
{
	Scene::Scene
	(
		IInputManager* _inputManager,
		IWindow* _window
	) 
		: m_inputManager(_inputManager)
		, m_window(_window)
		, m_entities()
		, m_processes()
	{
		
	}

	Scene::~Scene()
	{
		while (m_entities.size() > 0)
		{
			RemoveEntity(m_entities[0]);
		}

		delete m_inputManager;
		delete m_window;
	}

	IInputManager* Scene::GetInputManager()
	{
		return m_inputManager;
	}

	IWindow* Scene::GetWindow()
	{
		return m_window;
	}

	EntityPtr Scene::AddEntity(EntityPtr _entity)
	{
		m_entityToComponentMap[_entity->ID()] = new ComponentList();
		m_entities.push_back(_entity);
		TRIGGER_EVENT(EntityAdded, _entity);
		return _entity;
	}

	EntityPtr Scene::RemoveEntity(EntityPtr _entity)
	{
		assert(EntityIsInScene(_entity));

		TRIGGER_EVENT(EntityRemoved, _entity);

		auto components = m_entityToComponentMap[_entity->ID()];
		delete components;
		m_entityToComponentMap[_entity->ID()] = nullptr;
		
		m_entities.erase(find(m_entities.begin(), m_entities.end(), _entity));

		return _entity;
	}

	void Scene::AddComponentToEntity(ComponentPtr _component, EntityPtr _entity)
	{
		auto components = m_entityToComponentMap[_entity->ID()];
		assert(components);
		components->push_back(_component);

		TRIGGER_EVENT(ComponentAddedToEntity, _entity, _component);
	}

	void Scene::RemoveComponentFromEntity(ComponentPtr _component, EntityPtr _entity)
	{
		ComponentList components = GetComponentsForEntity(_entity);
		auto iter = std::find(components.begin(), components.end(), _component);
		assert(iter != components.end());
		components.erase(iter);

		TRIGGER_EVENT(ComponentRemovedFromEntity, _entity, _component);
	}

	bool Scene::EntityIsInScene(EntityPtr _entity)
	{
		return m_entityToComponentMap[_entity->ID()] != nullptr;
	}

	bool Scene::EntityHasComponentOfType(EntityPtr _entity, const type_info & _type)
	{
		ComponentList componentsForThisEntity =	GetComponentsForEntity(_entity);

		for (ComponentPtr componentPtr : componentsForThisEntity)
		{
			const std::type_info& componentType = componentPtr->TypeInfo();

			if (componentType == _type)
			{
				return true;
			}
		}
		return false;
	}

	void Scene::AddProcess(ProcessPtr _process)
	{
		assert(std::find(m_processes.begin(), m_processes.end(), _process) == m_processes.end());
		m_processes.push_back(_process);
		_process->BindToScene(this);
	}

	void Scene::RemoveProcess(ProcessPtr _process)
	{
		auto iter = std::find(m_processes.begin(), m_processes.end(), _process);
		assert(iter != m_processes.end());
		m_processes.erase(iter);
		_process->UnbindFromScene(this);
	}

	void Scene::Update(double _dt)
	{
		for (ProcessPtr process : m_processes)
		{
			process->Update(_dt);
		}
	}

	ComponentList const & Scene::GetComponentsForEntity(EntityPtr _entity)
	{
		return *m_entityToComponentMap[_entity->ID()];
	}

	EntityList const & Scene::Entities()
	{
		return m_entities;
	}
}