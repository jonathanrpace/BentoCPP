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

		_geometry->SwapHeightData();
		_geometry->SwapFluxData();

		SetTexture("s_heightData", &(_geometry->HeightDataRead()));
		SetTexture("s_fluxData", &(_geometry->FluxDataRead()));
		SetTexture("s_velocityData", &(_geometry->VelocityData()));
		SetTexture("s_mappingData", &(_geometry->MappingDataRead()));

		_geometry->SwapHeightData();
		_geometry->SwapFluxData();
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

			m_shader.VertexShader().SetTexture("s_diffuseMap", &(node->material->SomeTexture));

			node->geom->Draw();
		}
	}

	RenderPhase TerrainGPass::GetRenderPhase()
	{
		return eRenderPhase_G;
	}
}