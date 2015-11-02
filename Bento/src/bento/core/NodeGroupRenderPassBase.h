#pragma once

#include <bento/core/NodeGroup.h>
#include <bento/core/IRenderPass.h>

namespace bento
{
	template<typename T>
	class NodeGroupRenderPassBase 
		: public IRenderPass
	{
	public:
		NodeGroupRenderPassBase(std::string _name = "NodeGroupRenderPassBase") 
			: IRenderPass(_name)
			, m_nodeGroup()
		{
			
		}

		~NodeGroupRenderPassBase()
		{
			m_nodeGroup.Unbind();
		}

		// From IRenderPass
		virtual void BindToScene(bento::Scene & _scene) override
		{
			m_nodeGroup.Bind(&_scene);
		}

		// From IRenderPass
		virtual void UnbindFromScene(bento::Scene & _scene) override
		{
			m_nodeGroup.Unbind();
		}

	protected:
		NodeGroup<T> m_nodeGroup;
	};
}