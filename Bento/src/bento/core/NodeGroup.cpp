#include "bento/core/NodeGroup.h"

#include <utility>  // make_pair()

namespace bento
{
	template<typename T>
	NodeGroup<T>::NodeGroup() 
		: m_nodes()
		, m_entityToNodeMap()
		, m_scene(nullptr)
	{
		static_assert(std::is_base_of<INode, T>::value,	"Node type not derived from INode");
	}

	template<typename T>
	NodeGroup<T>::~NodeGroup()
	{
		Unbind();
	}

	////////////////////////////////////////////////////////////////////////////////
	// PUBLIC
	////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	void NodeGroup<T>::Bind(Scene* scene)
	{
		m_scene = scene;

		for (EntityPtr entity : m_scene->Entities())
		{
			AddIfMatch(entity);
		}

		m_scene->EntityAdded += OnEntityAddedDelegate;
		m_scene->EntityRemoved += OnEntityRemovedDelegate;
		m_scene->ComponentAddedToEntity += OnComponentAddedToEntityDelegate;
		m_scene->ComponentRemovedFromEntity += OnComponentRemovedFromEntityDelegate;
	}

	template<typename T>
	void NodeGroup<T>::Unbind()
	{
		if (m_scene == nullptr)
		{
			return;
		}

		m_scene->EntityAdded -= OnEntityAddedDelegate;
		m_scene->EntityRemoved -= OnEntityRemovedDelegate;
		m_scene->ComponentAddedToEntity -= OnComponentAddedToEntityDelegate;
		m_scene->ComponentRemovedFromEntity -= OnComponentRemovedFromEntityDelegate;

		for (T* nodePtr : m_nodes)
		{
			delete nodePtr;
		}
		m_nodes.clear();
		m_entityToNodeMap.clear();

		m_scene = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	// PRIVATE
	////////////////////////////////////////////////////////////////////////////////

	template<typename T>
	void NodeGroup<T>::AddIfMatch(EntityPtr entity)
	{
		if (m_entityToNodeMap.find(entity->ID()) != m_entityToNodeMap.end())
		{
			return;
		}

		auto nodeInfo = T::GetReflectionInfo();
		for (size_t i = 0; i < nodeInfo.GetMembersCount(); i++)
		{
			auto nodeMemberInfo = nodeInfo.GetMembers()[i];
			if (m_scene->EntityHasComponentOfType(entity, nodeMemberInfo.m_typeInfo) == false)
			{
				return;
			}
		}

		T* nodePtr = new T();

		ComponentList componentsForThisEntity =
			m_scene->GetComponentsForEntity(entity);

		for (size_t i = 0; i < nodeInfo.GetMembersCount(); i++)
		{
			auto member = nodeInfo.GetMembers()[i];
			const std::type_info& typeInfo = member.m_typeInfo;

			for (ComponentPtr componentPtr : componentsForThisEntity)
			{
				if (componentPtr->TypeInfo() == typeInfo)
				{
					// Create a pointer targeting the member of the node.
					void* ptr = (void*)((size_t)nodePtr + member.m_offset);

					// Assign the smart pointer to this location in memory
					Component* nakedPointer = componentPtr.get();
					*(Component**)ptr = nakedPointer;

					break;
				}
			}
		}

		m_entityToNodeMap.insert(std::make_pair(entity->ID(), nodePtr));
		m_nodes.insert(m_nodes.end(), nodePtr);

		TRIGGER_EVENT(NodeAdded, *nodePtr)
	}

	template<typename T>
	void NodeGroup<T>::RemoveIfMatch(EntityPtr entity)
	{
		auto entityToNodeIter = m_entityToNodeMap.find(entity->ID());
		if (entityToNodeIter == m_entityToNodeMap.end())
		{
			return;
		}

		ComponentList componentsForThisEntity =
			m_scene->GetComponentsForEntity(entity);

		auto nodeInfo = T::GetReflectionInfo();
		for (size_t i = 0; i < nodeInfo.GetMembersCount(); i++)
		{
			const std::type_info& typeInfo = nodeInfo.GetMembers()[i].m_typeInfo;

			if (m_scene->EntityHasComponentOfType(entity, typeInfo) == false)
			{
				T* nodePtr = m_entityToNodeMap[entity->ID()];
				m_nodes.remove(nodePtr);
				m_entityToNodeMap.erase(entityToNodeIter);

				TRIGGER_EVENT(NodeRemoved, *nodePtr)

				delete nodePtr;
			}
		}
	}

	template<typename T>
	void NodeGroup<T>::OnEntityAdded(EntityPtr entity)
	{
		AddIfMatch(entity);
	}

	template<typename T>
	void NodeGroup<T>::OnEntityRemoved(EntityPtr entity)
	{
		RemoveIfMatch(entity);
	}

	template<typename T>
	void NodeGroup<T>::OnComponentAddedToEntity(EntityPtr entity, ComponentPtr component)
	{
		AddIfMatch(entity);
	}

	template<typename T>
	void NodeGroup<T>::OnComponentRemovedFromEntity(EntityPtr entity, ComponentPtr component)
	{
		RemoveIfMatch(entity);
	}
}