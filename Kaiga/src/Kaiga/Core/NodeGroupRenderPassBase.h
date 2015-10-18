#pragma once

#include <kaiga.h>

namespace Kaiga
{
	template<typename T>
	class NodeGroupRenderPassBase :
		public IRenderPass
	{
	public:
		NodeGroupRenderPassBase() :
			m_nodeGroup()
		{
			
		}

		~NodeGroupRenderPassBase()
		{
			m_nodeGroup.Unbind();
		}

		// From IRenderPass
		virtual void BindToScene(Ramen::Scene & _scene)
		{
			m_nodeGroup.Bind(&_scene);
		}

		// From IRenderPass
		virtual void UnbindFromScene(Ramen::Scene & _scene)
		{
			m_nodeGroup.Unbind();
		}

	protected:
		NodeGroup<T> m_nodeGroup;
	};
}