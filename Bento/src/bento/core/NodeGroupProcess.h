#pragma once

// Disable warning message 4250 
// warning C4250: 'Derived': inherits 'bento::NodeGroupProcess<T>::bento::NodeGroupProcess<T>::Process' via dominance
// warning C4250: 'Derived': inherits 'bento::NodeGroupProcess<T>::bento::NodeGroupProcess<T>::UnbindFromScene' via dominance
#pragma warning( disable : 4250 )

#include <typeinfo>
#include <string>

#include <bento/core/NodeGroup.h>
#include <bento/core/Process.h>

namespace bento
{
	template<typename T>
	class NodeGroupProcess 
		: public virtual Process
	{
	public:
		NodeGroupProcess(std::string _name = "NodeGroupProcess", const std::type_info& _typeInfo = typeid(NodeGroupProcess<T>)) 
			: Process(_name, _typeInfo)
			, m_nodeGroup()
		{
			
		}

		~NodeGroupProcess()
		{
			m_nodeGroup.Unbind();
		}

		// From Process
		virtual void BindToScene(Scene * const _scene) override
		{
			Process::BindToScene(_scene);
			m_nodeGroup.Bind(m_scene);
		}

		// From Process
		virtual void UnbindFromScene(Scene * const _scene) override
		{
			Process::UnbindFromScene(_scene);
			m_nodeGroup.Unbind();
		}

	protected:
		NodeGroup<T> m_nodeGroup;
	};
}