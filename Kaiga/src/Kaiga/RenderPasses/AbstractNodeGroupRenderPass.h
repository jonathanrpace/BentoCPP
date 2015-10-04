#pragma once

#include <Ramen/Core/INode.h>

#include <Kaiga/Core/IRenderPass.h>
#include <Ramen/Core/Scene.h>
#include <Ramen/Core/NodeGroup.h>

namespace Kaiga
{
	template<typename T>
	class AbstractNodeGroupRenderPass
	{
	public:
		AbstractNodeGroupRenderPass() :
			m_nodeGroup()
		{
			
		}

		~AbstractNodeGroupRenderPass()
		{
			m_nodeGroup.Unbind();
		}

		virtual void BindToScene(Ramen::Scene & _scene)
		{
			m_nodeGroup.Bind(&_scene);
		}

		virtual void UnbindFromScene(Ramen::Scene & _scene)
		{
			m_nodeGroup.Unbind();
		}

	protected:
		NodeGroup<T> m_nodeGroup;
	};
}