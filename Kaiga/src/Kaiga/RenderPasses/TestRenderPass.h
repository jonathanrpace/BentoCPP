#pragma once

#include <Kaiga/Core/NodeGroupRenderPassBase.h>
#include <Kaiga/Core/ShaderBase.h>
#include <Kaiga/Core/GeometryBase.h>
#include <Kaiga/ShaderStages/SimpleVert.h>
#include <Kaiga/ShaderStages/TestFrag.h>
#include <Kaiga/Components/Transform.h>

namespace Kaiga
{
	struct TestShader : ShaderBase<SimpleVert, TestFrag>
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