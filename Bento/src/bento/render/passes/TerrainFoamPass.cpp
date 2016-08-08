#include "TerrainFoamPass.h"

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
	// Update shader
	////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	FoamParticleUpdateVert::FoamParticleUpdateVert()
		: ShaderStageBase("shaders/FoamParticleUpdate.vert", false)
	{}

	void FoamParticleUpdateVert::OnPreLink()
	{
		const char * varyings[] = { "out_position", "out_velocity" };
		GL_CHECK(glTransformFeedbackVaryings(m_programName, 2, varyings, GL_SEPARATE_ATTRIBS));
	}

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
		for (auto node : m_nodeGroup.Nodes())
		{
			TerrainGeometry& terrainGeom = *node->geom;
			FoamParticleGeom& particleGeom = *node->foamGeom;
			TerrainMaterial& material = *node->material;

			// Update
			{
				glBindProgramPipeline(GL_NONE);

				m_foamParticleUpdateShader.BindPerPass();
			
				if (particleGeom.Switch())
				{
					GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particleGeom.TransformFeedbackObjB()));
					GL_CHECK(glBindVertexArray(particleGeom.VertexArrayA()));
				}
				else
				{
					GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, particleGeom.TransformFeedbackObjA()));
					GL_CHECK(glBindVertexArray(particleGeom.VertexArrayB()));
				}

				GL_CHECK(glBeginTransformFeedback(GL_POINTS));
				GL_CHECK(glEnable(GL_RASTERIZER_DISCARD));

				auto& vertexShader = m_foamParticleUpdateShader.VertexShader();

				vertexShader.SetTexture("s_heightData", terrainGeom.HeightData().GetRead());
				vertexShader.SetTexture("s_velocityData", terrainGeom.VelocityData().GetRead());

				GL_CHECK(glDrawArrays(GL_POINTS, 0, particleGeom.NumParticles()));

				GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
				GL_CHECK(glEndTransformFeedback());
				glUseProgram(GL_NONE);

				particleGeom.Switch(!particleGeom.Switch());
			}

			// Draw
			{
				m_shader.BindPerPass();

				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				glPointSize(1.0f);

				RenderParams::SetModelMatrix(node->transform->matrix);

				GLuint vertexArray = particleGeom.Switch() ? particleGeom.VertexArrayA() : particleGeom.VertexArrayB();

				GL_CHECK(glBindVertexArray(vertexArray));

				m_shader.BindPerPass();
				m_shader.VertexShader().SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());

				m_shader.VertexShader().SetUniform("u_lightDir", -glm::euclidean(vec2(material.lightAltitude, material.lightAzimuth)));
				m_shader.VertexShader().SetUniform("u_lightIntensity", material.directLightIntensity);
				m_shader.VertexShader().SetUniform("u_ambientLightIntensity", material.ambientLightIntensity);

				//glEnable(GL_PROGRAM_POINT_SIZE);
				GL_CHECK(glDrawArrays(GL_POINTS, 0, particleGeom.NumParticles()));
				//glDisable(GL_PROGRAM_POINT_SIZE);

				glBindVertexArray(GL_NONE);
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				glDisable(GL_BLEND);
			}
		}
	}
}