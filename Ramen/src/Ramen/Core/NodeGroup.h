#pragma once

#include <list>
#include <map>
#include <typeinfo>
#include <utility>  // make_pair()

#include <Ramen/Core/Scene.h>
#include <Ramen/Core/INode.h>

namespace Ramen
{
	template<typename T> 
	class NodeGroup 
	{
	public:
		typedef std::list<T*> NodeList;

	private:

		// Members
		NodeList m_nodes;
		Scene* m_scene;
		int m_numNodeMembers;
		std::map<Entity, T*> m_entityToNodeMap;

		// Events / Delegates
		DEFINE_DELEGATE_1(D_OnEntityAdded, NodeGroup<T>::OnEntityAdded, Entity)
		void OnEntityAdded(Entity entity);
		DEFINE_DELEGATE_1(D_OnEntityRemoved, NodeGroup<T>::OnEntityRemoved, Entity)
		void OnEntityRemoved(Entity entity);
		DEFINE_DELEGATE_2(D_OnComponentAddedToEntity, NodeGroup<T>::OnComponentAddedToEntity, Entity, ComponentPtr)
		void OnComponentAddedToEntity(Entity entity, ComponentPtr component);
		DEFINE_DELEGATE_2(D_OnComponentRemovedFromEntity, NodeGroup<T>::OnComponentRemovedFromEntity, Entity, ComponentPtr)
		void OnComponentRemovedFromEntity(Entity entity, ComponentPtr component);

		// Functions
		void AddIfMatch(Entity entity);
		void RemoveIfMatch(Entity entity);

	public:
		
		NodeGroup();
		~NodeGroup();

		void Bind(Scene* scene);
		void Unbind();

		inline const NodeList& GetNodes() { return m_nodes; }

		DECLARE_EVENT(void, const T&) NodeAdded;
		DECLARE_EVENT(void, const T&) NodeRemoved;
	};
}

#include "NodeGroup.cpp"