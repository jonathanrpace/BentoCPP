#include "TerrainSteamPass.h"

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
	SteamParticleUpdateVert::SteamParticleUpdateVert()
		: ShaderStageBase("shaders/SteamParticleUpdate.vert", false)
	{}

	void SteamParticleUpdateVert::OnPreLink()
	{
		const char * varyings[] = { "out_position", "out_velocity" };
		GL_CHECK(glTransformFeedbackVaryings(m_programName, 2, varyings, GL_SEPARATE_ATTRIBS));
	}

	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainSteamVert::TerrainSteamVert()
		: ShaderStageBase("shaders/SteamParticle.vert")
	{
	}
	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainSteamFrag::TerrainSteamFrag()
		: ShaderStageBase("shaders/SteamParticle.frag")
	{
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainSteamPass::TerrainSteamPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainSteamPass))
		, RenderPass(eRenderPhase_Forward)
	{
	}

	void TerrainSteamPass::Advance(double _dt)
	{
		for (auto node : m_nodeGroup.Nodes())
		{
			TerrainGeometry& terrainGeom = *(node->terrainGeom);
			SteamParticleGeom& particleGeom = *(node->particleGeom);
			TerrainMaterial& material = *(node->material);

			// Update
			{
				glBindProgramPipeline(GL_NONE);

				m_updateShader.BindPerPass();
			
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

				auto vertexShader = m_updateShader.VertexShader();

				vertexShader.SetTexture("s_heightData", terrainGeom.HeightDataRead());
				vertexShader.SetTexture("s_smudgeData", terrainGeom.SmudgeDataRead());

				GL_CHECK(glDrawArrays(GL_POINTS, 0, particleGeom.NumParticles()));

				GL_CHECK(glDisable(GL_RASTERIZER_DISCARD));
				GL_CHECK(glEndTransformFeedback());
				glUseProgram(GL_NONE);

				particleGeom.Switch(!particleGeom.Switch());
			}

			// Draw
			{
				m_drawShader.BindPerPass();

				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);
				glEnable(GL_DEPTH_TEST);
				glPointSize(16.0f);

				RenderParams::SetModelMatrix(node->transform->matrix);

				GLuint vertexArray = particleGeom.Switch() ? particleGeom.VertexArrayA() : particleGeom.VertexArrayB();

				GL_CHECK(glBindVertexArray(vertexArray));

				m_drawShader.BindPerPass();
				m_drawShader.VertexShader().SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());

				m_drawShader.VertexShader().SetUniform("u_cameraPos", RenderParams::CameraPosition());
				m_drawShader.VertexShader().SetUniform("u_lightDir", -glm::euclidean(vec2(material.lightAltitude, material.lightAzimuth)));

				m_drawShader.VertexShader().SetTexture("s_miscData", terrainGeom.MiscDataRead());


				m_drawShader.FragmentShader().SetUniform("u_moltenColor", material.moltenColor);

				m_drawShader.FragmentShader().SetTexture("s_texture", material.steamTexture);

				//m_drawShader.VertexShader().SetUniform("u_lightDir", -glm::euclidean(vec2(material.lightAltitude, material.lightAzimuth)));
				//m_drawShader.VertexShader().SetUniform("u_lightIntensity", material.directLightIntensity);
				//m_drawShader.VertexShader().SetUniform("u_ambientLightIntensity", material.ambientLightIntensity);

				glEnable(GL_PROGRAM_POINT_SIZE);
				GL_CHECK(glDrawArrays(GL_POINTS, 0, particleGeom.NumParticles()));
				glDisable(GL_PROGRAM_POINT_SIZE);

				glBindVertexArray(GL_NONE);
				glDepthMask(GL_TRUE);
				//glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
			}
		}
	}
}