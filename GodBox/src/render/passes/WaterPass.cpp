#include "WaterPass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtx/polar_coordinates.hpp>
#include <glm/glm.hpp>

#include <glfw3.h>

// bento
#include <bento/core/Logging.h>
#include <bento/render/RenderParams.h>

// app
#include <render/RenderParams.h>
#include <render/eRenderPhase.h>

using namespace bento;

namespace godBox
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	WaterVert::WaterVert()
		: ShaderStageBase("shaders/Water.vert") 
	{
	}

	void WaterVert::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		// Flow
		float phase = fmodf( (float)glfwGetTime() * _material.waterFlowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f );
		float phaseB = fmodf( phase + 0.5f, 1.0f );
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform("u_phaseA", phaseA );
		SetUniform("u_phaseB", phaseB );
		SetUniform("u_phaseAlpha", alphaB );
		SetUniform("u_waterFlowOffset", _material.waterFlowOffset );
		SetUniform("u_waterFlowRepeat", _material.waterFlowRepeat );

		// Waves
		float waveTime = (float)glfwGetTime() * _material.waterWaveSpeed;
		SetUniform( "u_waveTime", waveTime );
		SetUniform( "u_waveLevels", _material.waterWaveLevels );
		SetUniform( "u_waveAmplitude", _material.waterWaveAmplitude );
		SetUniform( "u_waveFreqBase", _material.waterWaveFrquencyBase );
		SetUniform( "u_waveFreqScalar", _material.waterWaveFrquencyScalar );
		SetUniform( "u_waveRoughness", _material.waterWaveRoughness );

		SetUniform("u_mvpMatrix", bento::RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", bento::RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", bento::RenderParams::ViewMatrix());

		SetUniform("u_depthToReflect", _material.waterDepthToReflect);
		SetUniform("u_dissolvedDirtDensityScalar", _material.dissolvedDirtDesntiyScalar);

		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_velocityData", _geometry.VelocityData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_normalData", _geometry.NormalData().GetRead());
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());
		SetTexture("s_fluxData", _geometry.WaterFluxData().GetRead());
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	WaterFrag::WaterFrag()
		: ShaderStageBase("shaders/Water.frag") 
	{
	}

	void WaterFrag::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		// Flow
		float phase = fmodf( (float)glfwGetTime() * _material.waterFlowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f );
		float phaseB = fmodf( phase + 0.5f, 1.0f );
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform("u_phaseA", phaseA );
		SetUniform("u_phaseB", phaseB );
		SetUniform("u_phaseAlpha", alphaB );
		SetUniform("u_waterFlowOffset", _material.waterFlowOffset );
		SetUniform("u_waterFlowRepeat", _material.waterFlowRepeat );

		// Waves
		float waveTime = (float)glfwGetTime() * _material.waterWaveSpeed;
		SetUniform( "u_waveTime", waveTime );
		SetUniform( "u_waveLevels", _material.waterWaveLevels );
		SetUniform( "u_waveAmplitude", _material.waterWaveAmplitude );
		SetUniform( "u_waveFreqBase", _material.waterWaveFrquencyBase );
		SetUniform( "u_waveFreqScalar", _material.waterWaveFrquencyScalar );
		SetUniform( "u_waveRoughness", _material.waterWaveRoughness );

		SetUniform("u_waterColor", _material.waterColor);
		SetUniform("u_dirtColor", _material.dirtColor);
		SetUniform("u_indexOfRefraction", _material.waterIndexOfRefraction);
		SetUniform("u_specularPower", _material.waterSpecularPower);

		//
		SetUniform("u_depthToFilter", _material.waterDepthToFilter);
		SetUniform("u_depthToDiffuse", _material.waterDepthToDiffuse);

		// Foam
		SetUniform("u_foamRepeat", _material.foamRepeat);
		SetUniform("u_foamDistortStrength", _material.foamDistortStrength);
		SetUniform("u_foamAlphaStrength", _material.foamAlphaStrength);
		
		// Lighting
		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightDistance", _material.lightDistance);
		SetUniform("u_lightIntensity", _material.directLightIntensity);

		SetTexture("s_envMap", _material.envMap);
		SetUniform("u_ambientLightIntensity", _material.ambientLightIntensity);

		SetUniform("u_mvpMatrix", bento::RenderParams::ModelViewProjectionMatrix(), true);
		SetUniform("u_viewMatrix", bento::RenderParams::ViewMatrix());

		SetTexture("s_output", RenderParams::GetRenderTarget().ColorTexture());
		SetTexture("s_positionBuffer", RenderParams::GetRenderTarget().PositionTexture());
		SetTexture("s_foamMap", _material.foamTexture);
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	WaterPass::WaterPass(std::string _name)
		: NodeGroupProcess(_name, typeid(WaterPass))
		, RenderPass(godBox::eRenderPhase_Transparent)
	{
	}

	void WaterPass::Advance(double _dt)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			bento::RenderParams::SetModelMatrix(node->transform->matrix);
			
			auto& geom = *node->geom;
			auto& material = *node->material;

			m_shader.VertexShader().BindPerModel(geom, material);
			m_shader.FragmentShader().BindPerModel(geom, material);

			geom.Bind();
			geom.Draw();
		}

		glDepthMask(GL_TRUE);
	}
}