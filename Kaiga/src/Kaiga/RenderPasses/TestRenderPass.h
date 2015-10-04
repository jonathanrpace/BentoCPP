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
		public AbstractNodeGroupRenderPass<TestRenderPassNode>,
		public IRenderPass
	{
	public:
		TestRenderPass();
		virtual void Render();
	private:
		TestShader m_shader;

		// Inherited via IRenderPass
		virtual void BindToScene(Ramen::Scene & scene) override;
		virtual void UnbindFromScene(Ramen::Scene & scene) override;
		virtual RenderPhase GetRenderPhase() override;
	};
}