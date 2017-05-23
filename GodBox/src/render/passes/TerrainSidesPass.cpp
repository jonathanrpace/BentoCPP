#include "TerrainSidesPass.h"

#include <glfw3.h>

#include <glm/glm.hpp>

// bento
#include <bento/render/RenderParams.h>

// app
#include <render/eRenderPhase.h>

namespace godBox
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainSidesVert::TerrainSidesVert() 
		: ShaderStageBase("shaders/TerrainSides.vert") 
	{
	}

	void TerrainSidesVert::BindPerModel(TerrainSidesGeometry& _geometry, TerrainMaterial& _material)
	{
		// Matrices
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix() );

		// Uniforms
		SetUniform("u_cameraPos", RenderParams::CameraPosition());
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainSidesFrag::TerrainSidesFrag() 
		: ShaderStageBase("shaders/TerrainSides.frag") 
	{

	}

	void TerrainSidesFrag::BindPerModel(TerrainSidesGeometry& _geometry, TerrainMaterial& _material)
	{

	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainSidesPass::TerrainSidesPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainSidesPass))
		, RenderPass("TerrainSidesPass", eRenderPhase_Forward)
	{
	}

	void TerrainSidesPass::Advance(double _dt)
	{
		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			node->geom->Bind();
			m_shader.VertexShader().BindPerModel(*node->geom, *node->material);
			m_shader.FragmentShader().BindPerModel(*node->geom, *node->material);
			
			node->geom->Draw();
		}
	}
}