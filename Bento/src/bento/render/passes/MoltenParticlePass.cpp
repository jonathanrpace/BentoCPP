#include "MoltenParticlePass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bento/core/Logging.h>

#include <gl/glew.h>

#include <glm/gtx/polar_coordinates.hpp>
#include <glm/glm.hpp>

#include <glfw3.h>

#include <bento/core/Logging.h>
#include <bento/render/RenderParams.h>

namespace bento
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	MoltenParticleVert::MoltenParticleVert()
		: ShaderStageBase("shaders/MoltenParticle.vert")
	{
	}
	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	MoltenParticleFrag::MoltenParticleFrag()
		: ShaderStageBase("shaders/MoltenParticle.frag")
	{
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	MoltenParticlePass::MoltenParticlePass(std::string _name)
		: NodeGroupProcess(_name, typeid(MoltenParticlePass))
		, RenderPass(eRenderPhase_Forward)
	{
	}

	void MoltenParticlePass::Advance(double _dt)
	{
		m_shader.BindPerPass();

		//glEnable(GL_BLEND);
		//glBlendEquation(GL_FUNC_ADD);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_WRITEMASK);

		glPointSize(4.0f);

		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);

			GLuint vertexArray = node->moltenParticleGeom->Switch() ? node->moltenParticleGeom->VertexArrayA() : node->moltenParticleGeom->VertexArrayB();

			GL_CHECK(glBindVertexArray(vertexArray));

			m_shader.BindPerPass();
			m_shader.VertexShader().SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
			
			//glEnable(GL_PROGRAM_POINT_SIZE);
			GL_CHECK(glDrawArrays(GL_POINTS, 0, node->moltenParticleGeom->NumParticles()));
			//glDisable(GL_PROGRAM_POINT_SIZE);

			glBindVertexArray(GL_NONE);
		}
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_WRITEMASK);
		glDisable(GL_BLEND);
	}
}