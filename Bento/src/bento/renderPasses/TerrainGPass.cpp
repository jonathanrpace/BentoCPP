#include "TerrainGPass.h"

#include <bento/core/RenderParams.h>

namespace bento
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainGVert::TerrainGVert() 
		: ShaderStageBase("shaders/TerrainG.vert") 
	{
	}

	void TerrainGVert::BindPerModel(TerrainGeometry* _geometry)
	{
		SetAsActiveShader();
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelMatrix", RenderParams::ModelMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_normalModelViewMatrix", RenderParams::NormalModelViewMatrix());

		SetTexture("s_textureData0", &(_geometry->Texture0A()));
		SetTexture("s_textureData1", &(_geometry->Texture1A()));
		SetTexture("s_textureData2", &(_geometry->Texture2A()));
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainGFrag::TerrainGFrag() 
		: ShaderStageBase("shaders/TerrainG.frag") 
	{
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	void TerrainGPass::Render()
	{
		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);

			node->geom->Bind();
			m_shader.VertexShader().BindPerModel(node->geom);

			node->geom->Draw();
		}
	}

	RenderPhase TerrainGPass::GetRenderPhase()
	{
		return eRenderPhase_G;
	}
}