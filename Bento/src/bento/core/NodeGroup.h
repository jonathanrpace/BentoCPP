#pragma once

#include <list>
#include <map>
#include <typeinfo>
#include <utility>  // make_pair()

#include <bento/core/Scene.h>
#include <bento/core/Entity.h>
#include <bento/core/INode.h>

namespace bento
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
		std::map<int, T*> m_entityToNodeMap;

		// Events / Delegates
		DEFINE_EVENT_HANDLER_1(NodeGroup<T>, OnEntityAdded, EntityPtr, entity);
		DEFINE_EVENT_HANDLER_1(NodeGroup<T>, OnEntityRemoved, EntityPtr, entity);
		DEFINE_EVENT_HANDLER_2(NodeGroup<T>, OnComponentAddedToEntity, EntityPtr, entity, ComponentPtr, component);
		DEFINE_EVENT_HANDLER_2(NodeGroup<T>, OnComponentRemovedFromEntity, EntityPtr, entity, ComponentPtr, component);

		// Functions
		void AddIfMatch(EntityPtr entity);
		void RemoveIfMatch(EntityPtr entity);

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