#include "TerrainFoamPass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bento/core/Logging.h>

#include <gl/glew.h>

#include <glfw3.h>

#include <bento/core/Logging.h>
#include <bento/core/RenderParams.h>

namespace bento
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainFoamVert::TerrainFoamVert()
		: ShaderStageBase("shaders/FoamParticle.vert")
	{
	}
	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainFoamFrag::TerrainFoamFrag()
		: ShaderStageBase("shaders/FoamParticle.frag")
	{
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainFoamPass::TerrainFoamPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainFoamPass))
		, RenderPass(eRenderPhase_Forward)
	{
	}

	void TerrainFoamPass::Advance(double _dt)
	{
		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);

			GLuint vertexArray = node->foamGeom->Switch() ? node->foamGeom->VertexArrayA() : node->foamGeom->VertexArrayB();

			GL_CHECK(glBindVertexArray(vertexArray));

			m_shader.BindPerPass();
			m_shader.VertexShader().SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
			m_shader.VertexShader().SetTexture("s_rockData", &node->geom->RockDataRead());
			m_shader.VertexShader().SetTexture("s_waterData", &node->geom->WaterDataRead());
			

			glEnable(GL_PROGRAM_POINT_SIZE);
			GL_CHECK(glDrawArrays(GL_POINTS, 0, node->foamGeom->NumParticles()));
			glDisable(GL_PROGRAM_POINT_SIZE);

			glBindVertexArray(GL_NONE);
		}
	}
}