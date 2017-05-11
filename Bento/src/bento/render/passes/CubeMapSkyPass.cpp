#include "CubeMapSkyPass.h"

#include <bento/render/RenderParams.h>

namespace bento
{
	CubeMapSkyVert::CubeMapSkyVert()
		: ShaderStageBase("shaders/Skybox.vert") 
	{
	}

	CubeMapSkyFrag::CubeMapSkyFrag()
		: ShaderStageBase("shaders/CubeMapSkybox.frag") 
	{
	}

	CubeMapSkyPass::CubeMapSkyPass
	(
		std::string _name,	// ="CubeMapSkyPass"
		int _renderPhase	// = 0
	)
		: NodeGroupProcess(_name, typeid(CubeMapSkyPass))
		, RenderPass("CubeMapSkyPass", _renderPhase)
	{

	}

	void CubeMapSkyPass::Advance(double _dt)
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			auto& geom = *node->geom;
			auto& material = *node->material;

			auto vs = m_shader.VertexShader();
			auto fs = m_shader.FragmentShader();

			vs.SetUniform( "u_projMat", RenderParams::ProjectionMatrix() );
			vs.SetUniform( "u_normalViewMat", RenderParams::NormalViewMatrix() );

			fs.SetTexture( "s_envMap", material.envMap );
			fs.SetUniform( "u_intensity", material.intensity );
			fs.SetUniform( "u_mipLevel", material.mipLevel );

			geom.Bind();
			geom.Draw();
		}

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}
}