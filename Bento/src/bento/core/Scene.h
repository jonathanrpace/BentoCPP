#pragma once

// bento
#include <bento.h>
#include <event.h>
#include <bento/core/IInputManager.h>
#include <bento/core/IWindow.h>
#include <bento/core/Component.h>
#include <bento/core/Entity.h>
#include <bento/core/Process.h>

namespace bento
{
	typedef std::vector<ComponentPtr>	ComponentList;
	typedef std::vector<EntityPtr>		EntityList;
	typedef std::vector<ProcessPtr>		ProcessList;

	class Scene
	{
	public:
		Scene(IInputManager* _inputManager, IWindow* _window);
		~Scene();

		// Methods
		IInputManager* GetInputManager();
		IWindow* GetWindow();
		EntityPtr AddEntity(EntityPtr _entity);
		EntityPtr RemoveEntity(EntityPtr _entity);
		void AddComponentToEntity(ComponentPtr _component, EntityPtr _entity);
		void RemoveComponentFromEntity(ComponentPtr _component, EntityPtr _entity);
		bool EntityIsInScene(EntityPtr _entity);
		bool EntityHasComponentOfType(EntityPtr _entity, const type_info& _typeInfo);
		void AddProcess(ProcessPtr _process);
		void RemoveProcess(ProcessPtr _process);
		void Advance(double _dt);
		ComponentList const & GetComponentsForEntity(EntityPtr _entity);
		EntityList const & Entities();
		ProcessList const & Processes();

		template<typename T>
		std::shared_ptr<T> GetComponentForEntity(EntityPtr _entity)
		{
			ComponentList componentsForThisEntity = GetComponentsForEntity(_entity);

			const std::type_info& wantedType = typeid(T);

			for (ComponentPtr componentPtr : componentsForThisEntity)
			{
				const std::type_info& componentType = componentPtr->TypeInfo();

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
				const std::type_info& processType = processPtr->TypeInfo();

				if (processType == wantedType)
				{
					return std::static_pointer_cast<T>(processPtr);
				}
			}

			return nullptr;
		}

		// Events
		DECLARE_EVENT(void, EntityPtr) EntityAdded;
		DECLARE_EVENT(void, EntityPtr) EntityRemoved;
		DECLARE_EVENT(void, EntityPtr, ComponentPtr) ComponentAddedToEntity;
		DECLARE_EVENT(void, EntityPtr, ComponentPtr) ComponentRemovedFromEntity;

	private:
		IInputManager* m_inputManager;
		IWindow* m_window;
		EntityList m_entities;
		ProcessList m_processes;
		ComponentList* m_entityToComponentMap[4096];
	};
}