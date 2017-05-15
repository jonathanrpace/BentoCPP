#include "SteamPass.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <gl/glew.h>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/glm.hpp>
#include <glfw3.h>
#include <imgui.h>

// bento
#include <bento/core/Logging.h>
#include <bento/render/RenderParams.h>

// app
#include <render/eRenderPhase.h>
#include <render/RenderParams.h>

namespace godBox
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
		const char * varyings[] = { "out_data0", "out_data1", "out_data2" };
		GL_CHECK(glTransformFeedbackVaryings(m_programName, 3, varyings, GL_SEPARATE_ATTRIBS));
	}

	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	SteamVert::SteamVert()
		: ShaderStageBase("shaders/SteamParticle.vert")
	{
	}
	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	SteamFrag::SteamFrag()
		: ShaderStageBase("shaders/SteamParticle.frag")
	{
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	SteamPass::SteamPass(std::string _name)
		: NodeGroupProcess(_name, typeid(SteamPass))
		, RenderPass("SteamPass", eRenderPhase_Transparent)
		, SerializableBase("SteamPass")
	{
		SERIALIZABLE(spawnDelay,				vec2(0.0f, 0.5f));
		SERIALIZABLE(spawnThreshold,			vec2(0.1f, 0.5f));

		SERIALIZABLE(spawnVelocityMin,			vec3(0.0f,0.0f,0.0f));
		SERIALIZABLE(spawnVelocityMax,			vec3(0.0f,0.0f,0.0f));

		SERIALIZABLE(life,						vec2(1.0f, 3.0f));

		SERIALIZABLE(positionAccelerationMin,	vec3(0.0f,0.0f,0.0f));
		SERIALIZABLE(positionAccelerationMax,	vec3(0.0f,0.0f,0.0f));
		SERIALIZABLE(positionDamping,			vec2(0.0f, 0.0f));

		SERIALIZABLE(sizeAcceleration,			vec2(0.0f,0.0f));
		SERIALIZABLE(sizeDamping,				vec2(0.0f,0.0f));

		SERIALIZABLE(spawnSize,				0.05f);

		SERIALIZABLE(fadeInTime,				0.25f);
		SERIALIZABLE(fadeOutTime,				0.25f);

		ResetToDefaults();
	}

	void SteamPass::AddUIElements()
	{
		ImGui::Spacing();
		ImGui::Text("Spawning");
		ImGui::SliderFloat2("Threshold min/max", &spawnThreshold.x, 0.0f, 1.0f);
		ImGui::SliderFloat2("Delay min/max", &spawnDelay.x, 0.0f, 1.0f);
		ImGui::SliderFloat2("Life min/max", &life.x, 0.0f, 8.0f);
		ImGui::SliderFloat3("Velocity min", &spawnVelocityMin.x, -0.001f, 0.001f, "%.5f");
		ImGui::SliderFloat3("Velocity max", &spawnVelocityMax.x, -0.001f, 0.001f, "%.5f");
		ImGui::SliderFloat("Size", &spawnSize, 0.0f, 0.2f);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Position motion");
		ImGui::SliderFloat3("Min#pos", &positionAccelerationMin.x, -0.0001f, 0.0001f, "%.6f");
		ImGui::SliderFloat3("Max#pos", &positionAccelerationMax.x, -0.0001f, 0.0001f, "%.6f");
		ImGui::SliderFloat2("Damping min/max#pos", &positionDamping.x, 0.0, 1.0);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Size motion");
		ImGui::SliderFloat2("Acceleration min/max#scale", &sizeAcceleration.x, -0.0001f, 0.0001f, "%.6f");
		ImGui::SliderFloat2("Damping min/max#scale", &sizeDamping.x, 0.0, 1.0);
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Alpha");
		ImGui::SliderFloat("Fade in time#alpha", &fadeInTime, 0.0f, 1.0f);
		ImGui::SliderFloat("Fade out time#alpha", &fadeOutTime, 0.0f, 1.0f);
		ImGui::Spacing();

		if (ImGui::Button("Reset"))
		{
			ResetToDefaults();
		}
		if (ImGui::Button("Save"))
		{
			FlushChanges();
		}
	}

	void SteamPass::Advance(double _dt)
	{
		for (auto node : m_nodeGroup.Nodes())
		{
			auto& terrainGeom = *node->terrainGeom;
			auto& particleGeom = *node->particleGeom;
			auto& terrainMaterial = *node->terrainMaterial;
			auto& smokeMaterial = *node->smokeMaterial;

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

				auto& vs = m_updateShader.VertexShader();

				// Textures
				vs.SetTexture("s_heightData", terrainGeom.HeightData().GetRead());
				vs.SetTexture("s_miscData", terrainGeom.MiscData().GetRead());

				// Uniforms
				vs.SetUniform("u_dt", (float)_dt);
				vs.SetUniform("u_spawnThreshold", spawnThreshold);
				vs.SetUniform("u_spawnDelay", spawnDelay);
				vs.SetUniform("u_life", life);
				vs.SetUniform("u_spawnVelocityMin", spawnVelocityMin);
				vs.SetUniform("u_spawnVelocityMax", spawnVelocityMax);
				vs.SetUniform("u_spawnSize", spawnSize);

				vs.SetUniform("u_positionAccelerationMin", positionAccelerationMin);
				vs.SetUniform("u_positionAccelerationMax", positionAccelerationMax);
				vs.SetUniform("u_positionDamping", positionDamping);

				vs.SetUniform("u_sizeAcceleration", sizeAcceleration);
				vs.SetUniform("u_sizeDamping", sizeDamping);

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
				glDepthFunc(GL_LESS);

				bento::RenderParams::SetModelMatrix(node->transform->matrix);

				GLuint vertexArray = particleGeom.Switch() ? particleGeom.VertexArrayA() : particleGeom.VertexArrayB();

				GL_CHECK(glBindVertexArray(vertexArray));

				m_drawShader.BindPerPass();
				SteamVert& vs = m_drawShader.VertexShader();

				//vs.SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
				vs.SetUniform("u_modelViewMatrix", bento::RenderParams::ModelViewMatrix());
				vs.SetUniform("u_projMatrix", bento::RenderParams::ProjectionMatrix());

				vs.SetUniform("u_cameraPos", bento::RenderParams::CameraPosition());
				vs.SetUniform("u_frameBufferSize", bento::RenderParams::BackBufferSize());

				vs.SetUniform("u_fadeInTime", fadeInTime);
				vs.SetUniform("u_fadeOutTime", fadeOutTime);

				//vs.SetUniform("u_lightDir", -glm::euclidean(vec2(terrainMaterial.lightAltitude, terrainMaterial.lightAzimuth)));
				//vs.SetUniform("u_moltenColor", terrainMaterial.moltenColor);

				//vs.SetTexture("s_miscData", terrainGeom.MiscData().GetRead());

				SteamFrag& fs = m_drawShader.FragmentShader();

				fs.SetTexture( "s_positionBuffer", RenderParams::GetRenderTarget().PositionTexture());
				fs.SetTexture( "s_outputBuffer", RenderParams::GetRenderTarget().ColorTexture());
				fs.SetTexture( "s_texture", smokeMaterial.texture);
				fs.SetTexture( "s_envMap", terrainMaterial.irrMap);
				fs.SetTexture( "s_blurredColorBuffer", RenderParams::GetRenderTarget().BlurredColorTextureB());

				fs.SetUniform( "u_cameraPos", bento::RenderParams::CameraPosition() );
				fs.SetUniform( "u_cameraForward", bento::RenderParams::CameraForward() );
				fs.SetUniform( "u_cameraRight", bento::RenderParams::CameraRight() );
				fs.SetUniform( "u_cameraUp", bento::RenderParams::CameraUp() );
				fs.SetUniform( "u_albedo", smokeMaterial.baseColor);
				fs.SetUniform( "u_filterColor", smokeMaterial.filterColor);
				fs.SetUniform( "u_filterStrength", smokeMaterial.filterStrength);
				fs.SetUniform( "u_density", smokeMaterial.density);

				fs.SetUniform( "u_ambientLightIntensity", terrainMaterial.ambientLightIntensity);

				//m_drawShader.FragmentShader().SetTexture( "s_texture", material.smokeTexture );
				//m_drawShader.FragmentShader().SetUniform("u_moltenColor", terrainMaterial.moltenColor);
				//vs.SetUniform("u_lightDir", -glm::euclidean(vec2(terrainMaterial.lightAltitude, terrainMaterial.lightAzimuth)));
				//vs.SetUniform("u_lightIntensity", terrainMaterial.directLightIntensity);
				//vs.SetUniform("u_ambientLightIntensity", terrainMaterial.ambientLightIntensity);

				glEnable(GL_PROGRAM_POINT_SIZE);
				GL_CHECK(glDrawArrays(GL_POINTS, 0, particleGeom.NumParticles()));
				glDisable(GL_PROGRAM_POINT_SIZE);

				glBindVertexArray(GL_NONE);
				//glDepthMask(GL_TRUE);
				//glEnable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
			}
		}
	}
}