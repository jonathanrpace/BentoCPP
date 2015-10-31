#pragma once

// std
#include <assert.h>
#include <vector>
#include <stack>
#include <memory>

// bento
#include <event.h>
#include <bento/core/IInputManager.h>
#include <bento/core/IWindow.h>
#include <bento/core/IComponent.h>
#include <bento/core/IProcess.h>

namespace bento
{
	typedef int Entity;
	typedef std::shared_ptr<IComponent>	ComponentPtr;
	typedef std::shared_ptr<IProcess>	ProcessPtr;
	typedef std::vector<ComponentPtr>	ComponentList;
	typedef std::vector<Entity>			EntityList;
	typedef std::vector<ProcessPtr>		ProcessList;

	class Scene
	{
	public:
		Scene(IInputManager* _inputManager, IWindow* _window);
		~Scene();

		// Methods
		IInputManager* GetInputManager();
		IWindow* GetWindow();
		Entity CreateEntity();
		void DestroyEntity(Entity _entity);
		void AddComponentToEntity(ComponentPtr _component, Entity _entity);
		void RemoveComponentFromEntity(ComponentPtr _component, Entity _entity);
		bool DoesEntityExist(Entity _entity);
		bool EntityHasComponentOfType(Entity _entity, const type_info& _typeInfo);
		void AddProcess(ProcessPtr _process);
		void RemoveProcess(ProcessPtr _process);
		void Update(double _dt);
		ComponentList const & GetComponentsForEntity(Entity _entity);
		EntityList const & Entities();

		template<typename T>
		std::shared_ptr<T> GetComponentForEntity(Entity _entity)
		{
			ComponentList componentsForThisEntity = GetComponentsForEntity(_entity);

			const std::type_info& wantedType = typeid(T);

			for (ComponentPtr componentPtr : componentsForThisEntity)
			{
				const std::type_info& componentType = componentPtr->typeInfo();

				if (componentType == wantedType)
				{
					return std::static_pointer_cast<T>(componentPtr);
				}
			}

			return nullptr;
		}

		template<typename T>
		std::shared_ptr<T> GetProcess()
		{
			const std::type_info& wantedType = typeid(T);

			for (ProcessPtr processPtr : m_processes)
			{
				const std::type_info& processType = processPtr->typeInfo();

				if (processType == wantedType)
				{
					return std::static_pointer_cast<T>(processPtr);
				}
			}

			return nullptr;
		}

		// Events
		DECLARE_EVENT(void, Entity) EntityAdded;
		DECLARE_EVENT(void, Entity) EntityRemoved;
		DECLARE_EVENT(void, Entity, ComponentPtr) ComponentAddedToEntity;
		DECLARE_EVENT(void, Entity, ComponentPtr) ComponentRemovedFromEntity;	

	private:
		IInputManager* m_inputManager;
		IWindow* m_window;
		EntityList m_entities;
		ProcessList m_processes;
		ComponentList* m_entityToComponentMap[4096];
		std::stack<Entity> m_entityPool;
		int m_maxEntityID;
	};
}