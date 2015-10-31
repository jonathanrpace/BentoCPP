#pragma once

#include <bento/core/NodeGroupRenderPassBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/GeometryBase.h>
#include <bento/shaderStages/SimpleVert.h>
#include <bento/shaderStages/TestFrag.h>
#include <bento/components/Transform.h>

namespace bento
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
		public bento::SharedObject<TestRenderPass>,
		public NodeGroupRenderPassBase<TestRenderPassNode>
	{
	public:
		virtual void Render();
	private:
		TestShader m_shader;

		virtual RenderPhase GetRenderPhase() override;
	};
}