#include <utility>  // make_pair()

using Ramen::NodeGroup;
using Ramen::Scene;
using Ramen::Entity;

template<typename T>
NodeGroup<T>::NodeGroup() :
	m_nodes(),
	m_entityToNodeMap(),
	m_scene(nullptr)
{
	static_assert(std::is_base_of<INode, T>::value, 
		"Node type not derived from INode");
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

	for (Entity entity : m_scene->Entities())
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
void Ramen::NodeGroup<T>::AddIfMatch(Entity entity)
{
	if (m_entityToNodeMap.find(entity) != m_entityToNodeMap.end())
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

		for (ComponentPtr component : componentsForThisEntity)
		{
			if (component->typeInfo() == typeInfo)
			{
				// Create a pointer targeting the member of the node.
				ComponentPtr* ptr = (ComponentPtr*)nodePtr + member.m_offset;

				// Assign the smart pointer to this location in memory
				**ptr = *component;
			}
		}
	}

	m_entityToNodeMap.insert(std::make_pair(entity, nodePtr));
	m_nodes.insert(m_nodes.end(), nodePtr);

	TRIGGER_EVENT(NodeAdded, *nodePtr)
}

template<typename T>
void Ramen::NodeGroup<T>::RemoveIfMatch(Entity entity)
{
	auto entityToNodeIter = m_entityToNodeMap.find(entity);
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
			T* nodePtr = m_entityToNodeMap[entity];
			m_nodes.remove(nodePtr);
			m_entityToNodeMap.erase(entityToNodeIter);

			TRIGGER_EVENT(NodeRemoved, *nodePtr)

			delete nodePtr;
		}
	}
}

template<typename T>
void NodeGroup<T>::OnEntityAdded(Entity entity)
{
	AddIfMatch(entity);
}

template<typename T>
void Ramen::NodeGroup<T>::OnEntityRemoved(Entity entity)
{
	RemoveIfMatch(entity);
}

template<typename T>
void Ramen::NodeGroup<T>::OnComponentAddedToEntity(Entity entity, ComponentPtr component)
{
	AddIfMatch(entity);
}

template<typename T>
void Ramen::NodeGroup<T>::OnComponentRemovedFromEntity(Entity entity, ComponentPtr component)
{
	RemoveIfMatch(entity);
}
