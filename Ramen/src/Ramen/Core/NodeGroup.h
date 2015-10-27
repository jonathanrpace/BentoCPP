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
		DEFINE_EVENT_HANDLER_1(NodeGroup<T>, OnEntityAdded, Entity, entity);
		DEFINE_EVENT_HANDLER_1(NodeGroup<T>, OnEntityRemoved, Entity, entity);
		DEFINE_EVENT_HANDLER_2(NodeGroup<T>, OnComponentAddedToEntity, Entity, entity, ComponentPtr, component);
		DEFINE_EVENT_HANDLER_2(NodeGroup<T>, OnComponentRemovedFromEntity, Entity, entity, ComponentPtr, component);

		// Functions
		void AddIfMatch(Entity entity);
		void RemoveIfMatch(Entity entity);

	public:
		
		NodeGroup();
		~NodeGroup();

		void Bind(Scene* scene);
		void Unbind();

		inline const NodeList& Nodes() { return m_nodes; }

		DECLARE_EVENT(void, const T&) NodeAdded;
		DECLARE_EVENT(void, const T&) NodeRemoved;
	};
}

#include "NodeGroup.cpp"