#include "GPass.h"

#include <bento/render/RenderParams.h>

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

	GPass::GPass(std::string _name)
		: NodeGroupProcess<GPassNode>(_name, typeid(GPass))
		, RenderPass(eRenderPhase_G)
	{

	}

	void GPass::Advance(double _dt)
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