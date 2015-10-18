#pragma once

#include <kaiga.h>

#include <Kaiga/Components/Transform.h>

#include <Kaiga/Shaders/SimpleVert.h>
#include <Kaiga/Shaders/TestFrag.h>

namespace Kaiga
{
	class TestShader : public ShaderBase<SimpleVert, TestFrag>
	{};

	DEFINE_NODE_2
	(
		TestRenderPassNode,
		GeometryBase, geom,
		Transform, transform
	)

	class TestRenderPass :
		public Ramen::SharedObject<TestRenderPass>,
		public NodeGroupRenderPassBase<TestRenderPassNode>
	{
	public:
		virtual void Render();
	private:
		TestShader m_shader;

		virtual RenderPhase GetRenderPhase() override;
	};
}