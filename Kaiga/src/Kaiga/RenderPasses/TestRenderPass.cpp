#include "TestRenderPass.h"

namespace Kaiga
{
	void TestRenderPass::Render()
	{
		m_shader.BindPerPass();

		for (auto node : m_nodeGroup.GetNodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			m_shader.GetVertexShader().BindPerModel();
			node->geom->Bind();
			node->geom->Draw();
		}
	}

	RenderPhase TestRenderPass::GetRenderPhase()
	{
		return Kaiga::eRenderPhase_G;
	}
}