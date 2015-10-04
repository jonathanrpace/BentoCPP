#include "TestRenderPass.h"

Kaiga::TestRenderPass::TestRenderPass() :
	m_shader()
{
}

void Kaiga::TestRenderPass::Render()
{
	m_shader.BindPerPass();

	for ( auto node : m_nodeGroup.GetNodes() )
	{
		node->geom.Bind();
		node->geom.Draw();
	}
}

void Kaiga::TestRenderPass::BindToScene(Ramen::Scene & scene)
{
}

void Kaiga::TestRenderPass::UnbindFromScene(Ramen::Scene & scene)
{
}

Kaiga::RenderPhase Kaiga::TestRenderPass::GetRenderPhase()
{
	return Kaiga::eRenderPhase_G;
}