#include "GPass.h"

#include <bento/core/RenderParams.h>

namespace bento
{
	GVert::GVert() : ShaderStageBase("shaders/G.vert") {}
	void GVert::BindPerModel()
	{
		SetAsActiveShader();
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_normalModelViewMatrix", RenderParams::NormalModelViewMatrix());
	}

	GFrag::GFrag() : ShaderStageBase("shaders/G.frag") {}
	void GFrag::BindPerMaterial(StandardMaterial* _material)
	{
		SetAsActiveShader();
		SetUniform("u_reflectivity", _material->Reflectivity());
		SetUniform("u_roughness", _material->Roughness());
		SetUniform("u_emissive", _material->Emissive());
		SetUniform("u_albedo", _material->Diffuse());	// TODO - degamma
	}
	
	RenderPhase GPass::GetRenderPhase()
	{
		return eRenderPhase_G;
	}

	void GPass::Render()
	{
		m_shader.BindPerPass();
		for( auto node : m_nodeGroup.Nodes() )
		{
			RenderParams::SetModelMatrix(node->transform->matrix);

			m_shader.VertexShader().BindPerModel();
			m_shader.FragmentShader().BindPerMaterial(node->material);
			node->geom->Bind();
			node->geom->Draw();
		}
	}
}