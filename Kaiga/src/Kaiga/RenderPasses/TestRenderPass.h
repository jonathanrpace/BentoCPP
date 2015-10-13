#pragma once

#include <kaiga.h>

#include <Kaiga/RenderPasses/AbstractNodeGroupRenderPass.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Shaders/TestShader.h>

namespace Kaiga
{
	DEFINE_NODE_2
	(
		TestRenderPassNode,
		AbstractGeometry, geom,
		Transform, transform
	)

	class TestRenderPass :
		public Ramen::SharedObject<TestRenderPass>,
		public AbstractNodeGroupRenderPass<TestRenderPassNode>
	{
	public:
		virtual void Render();
	private:
		TestShader m_shader;

		virtual RenderPhase GetRenderPhase() override;
	};
}