#include "TestRenderPass.h"

namespace bento
{
	void TestRenderPass::Render()
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

	RenderPhase TestRenderPass::GetRenderPhase()
	{
		return bento::eRenderPhase_G;
	}
}