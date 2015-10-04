#include "Scene.h"

namespace Ramen
{
	Scene::Scene() :
		m_entities(),
		m_processes(),
		m_entityPool(),
		m_maxEntityID(0)
	{
		
	}

	Scene::~Scene()
	{
		while (m_entities.size() > 0)
		{
			DestroyEntity(m_entities[0]);
		}
	}

	Entity Scene::CreateEntity()
	{
		Entity entity;
		if (m_entityPool.size() > 0)
		{
			entity = m_entityPool.top();
			m_entityPool.pop();
		}
		else
		{
			entity = m_maxEntityID++;
		}

		auto components = new ComponentList();
		m_entityToComponentMap[entity] = components;

		m_entities.push_back(entity);

		TRIGGER_EVENT(EntityAdded, entity);

		return entity;
	}

	void Scene::DestroyEntity(Entity _entity)
	{
		assert(DoesEntityExist(_entity));

		TRIGGER_EVENT(EntityRemoved, _entity);

		auto components = m_entityToComponentMap[_entity];
		delete components;
		m_entityToComponentMap[_entity] = nullptr;
		
		m_entities.erase(find(m_entities.begin(), m_entities.end(), _entity));
	}

	void Scene::AddComponentToEntity(ComponentPtr _component, Entity _entity)
	{
		auto components = m_entityToComponentMap[_entity];
		components->push_back(_component);

		TRIGGER_EVENT(ComponentAddedToEntity, _entity, _component);
	}

	void Scene::RemoveComponentFromEntity(ComponentPtr _component, Entity _entity)
	{
		ComponentList components = GetComponentsForEntity(_entity);
		auto iter = std::find(components.begin(), components.end(), _component);
		assert(iter != components.end());
		components.erase(iter);

		TRIGGER_EVENT(ComponentRemovedFromEntity, _entity, _component);
	}

	bool Scene::DoesEntityExist(Entity _entity)
	{
		return m_entityToComponentMap[_entity] != nullptr;
	}

	bool Scene::EntityHasComponentOfType(Entity _entity, const type_info & _type)
	{
		ComponentList componentsForThisEntity =	GetComponentsForEntity(_entity);

		for (ComponentPtr componentPtr : componentsForThisEntity)
		{
			const std::type_info& componentType = componentPtr->typeInfo();

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

	ComponentList const & Scene::GetComponentsForEntity(Entity _entity)
	{
		return *m_entityToComponentMap[_entity];
	}

	EntityList const & Scene::Entities()
	{
		return m_entities;
	}
}