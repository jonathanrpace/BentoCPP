#pragma once

#include <Ramen/Core/SharedObject.h>
#include <Kaiga/RenderPasses/AbstractNodeGroupRenderPass.h>
#include <Kaiga/Components/Geom/ScreenQuadGeometry.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Shaders/TestShader.h>

namespace Kaiga
{
	DEFINE_NODE_2
	(
		TestRenderPassNode,
		ScreenQuadGeometry, geom,
		Transform, transform
	)

	class TestRenderPass :
		public Ramen::SharedObject<TestRenderPass>,
		public AbstractNodeGroupRenderPass<TestRenderPassNode>
	{
	public:
		TestRenderPass();
		virtual void Render();
	private:
		TestShader m_shader;

		virtual RenderPhase GetRenderPhase() override;
	};
}