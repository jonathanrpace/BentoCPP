#pragma once

#include <string>

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/RenderPass.h>
#include <bento/geom/Geometry.h>
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
		Geometry, geom,
		Transform, transform
	)

	class TestRenderPass
		: public NodeGroupProcess<TestRenderPassNode>
		, public RenderPass
		, public SharedObject<TestRenderPass>
	{
	public:
		TestRenderPass(std::string _name = "TestRenderPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TestShader m_shader;
	};
}