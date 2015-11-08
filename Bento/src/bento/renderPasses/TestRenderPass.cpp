#include "TestRenderPass.h"

namespace bento
{
	TestRenderPass::TestRenderPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TestRenderPass))
		, RenderPass(eRenderPhase_G)
	{

	}

	void TestRenderPass::Advance(double _dt)
	{
		m_shader.BindPerPass();

		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			m_shader.VertexShader().BindPerModel();
			node->geom->Bind();
			node->geom->Draw();
		}
	}
}