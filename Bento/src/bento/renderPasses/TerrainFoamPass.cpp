#include "TerrainFoamPass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bento/core/Logging.h>

#include <gl/glew.h>

#include <glm/gtx/polar_coordinates.hpp>
#include <glm/glm.hpp>

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

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);

			GLuint vertexArray = node->foamGeom->Switch() ? node->foamGeom->VertexArrayA() : node->foamGeom->VertexArrayB();

			GL_CHECK(glBindVertexArray(vertexArray));

			m_shader.BindPerPass();
			m_shader.VertexShader().SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
			m_shader.VertexShader().SetTexture("s_rockData", &node->geom->RockDataRead());
			m_shader.VertexShader().SetTexture("s_waterData", &node->geom->WaterDataRead());

			m_shader.VertexShader().SetUniform("u_lightDir", -glm::euclidean(vec2(node->material->lightAltitude, node->material->lightAzimuth)));
			m_shader.VertexShader().SetUniform("u_lightIntensity", node->material->directLightIntensity);
			m_shader.VertexShader().SetUniform("u_ambientLightIntensity", node->material->ambientLightIntensity);

			glEnable(GL_PROGRAM_POINT_SIZE);
			GL_CHECK(glDrawArrays(GL_POINTS, 0, node->foamGeom->NumParticles()));
			glDisable(GL_PROGRAM_POINT_SIZE);

			glBindVertexArray(GL_NONE);
		}

		glDisable(GL_BLEND);
	}
}