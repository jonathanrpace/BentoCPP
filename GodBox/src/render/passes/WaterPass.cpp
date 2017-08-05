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

	void WaterVert::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _terrainMaterial, WaterMaterial& _waterMaterial)
	{
		// Samplers
		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_fluidVelocity", _geometry.MoltenFluxData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_normalData", _geometry.NormalData());
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());
		SetTexture("s_fluxData", _geometry.WaterFluxData().GetRead());

		// Matrices
		SetUniform("u_mvpMatrix", bento::RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", bento::RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", bento::RenderParams::ViewMatrix());

		// Depth
		SetUniform("u_localDepthMip", _waterMaterial.localDepthMip);
		SetUniform("u_localDepthScalar", _waterMaterial.localDepthScalar);
		SetUniform("u_depthToReflect", _waterMaterial.depthToReflection);

		// Flow
		float phase = fmodf( (float)glfwGetTime() * _waterMaterial.flowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f );
		float phaseB = fmodf( phase + 0.5f, 1.0f );
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;
		SetUniform("u_phaseA", phaseA );
		SetUniform("u_phaseB", phaseB );
		SetUniform("u_phaseAlpha", alphaB );
		SetUniform("u_waterFlowOffset", _waterMaterial.flowOffset );

		// Waves
		float waveTime = (float)glfwGetTime() * _waterMaterial.waveSpeed;
		SetUniform( "u_waveTime", waveTime );
		SetUniform( "u_waveLevels", _waterMaterial.waveLevels );
		SetUniform( "u_waveAmplitude", _waterMaterial.waveAmplitude );
		SetUniform( "u_waveFreqBase", _waterMaterial.waveFrquencyBase );
		SetUniform( "u_waveFreqScalar", _waterMaterial.waveFrquencyScalar );
		SetUniform( "u_waveRoughness", _waterMaterial.waveRoughness );
		SetUniform( "u_waveChoppy", _waterMaterial.waveChoppy );
		SetUniform( "u_waveChoppyEase", _waterMaterial.waveChoppyEase );

		// Dirt
		SetUniform("u_dirtScalar", _waterMaterial.dirtScalar);
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	WaterFrag::WaterFrag()
		: ShaderStageBase("shaders/Water.frag") 
	{
	}

	void WaterFrag::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _terrainMaterial, WaterMaterial& _waterMaterial)
	{
		// Samplers
		SetTexture("s_output", RenderParams::GetRenderTarget().ColorTexture());
		SetTexture("s_positionBuffer", RenderParams::GetRenderTarget().PositionTexture());
		SetTexture("s_normalBuffer", RenderParams::GetRenderTarget().NormalTexture());
		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_foamMap", _waterMaterial.foamTexture);
		SetTexture("s_envMap", _terrainMaterial.envMap);
		SetTexture("s_irrMap", _terrainMaterial.irrMap);
		SetTexture("s_moltenGradient", _terrainMaterial.moltenGradient);
		
		// Matrices
		SetUniform("u_mvpMatrix", bento::RenderParams::ModelViewProjectionMatrix(), true);
		SetUniform("u_viewMatrix", bento::RenderParams::ViewMatrix());

		// Material 
		SetUniform("u_filterColor", _waterMaterial.filterColor);
		SetUniform("u_scatterColor", _waterMaterial.scatterColor);
		SetUniform("u_indexOfRefraction", _waterMaterial.indexOfRefraction);
		SetUniform("u_reflectivity", _waterMaterial.reflectivity);
		SetUniform("u_depthToFilter", _waterMaterial.depthToFilter);
		SetUniform("u_depthToDiffuse", _waterMaterial.depthToDiffuse);
		SetUniform("u_depthPower", _waterMaterial.depthPower);
		SetUniform("u_localDepthValue", _waterMaterial.localDepthValue);

		// Flow
		float phase = fmodf( (float)glfwGetTime() * _waterMaterial.flowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f );
		float phaseB = fmodf( phase + 0.5f, 1.0f );
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform("u_phaseA", phaseA );
		SetUniform("u_phaseB", phaseB );
		SetUniform("u_phaseAlpha", alphaB );
		SetUniform("u_waterFlowOffset", _waterMaterial.flowOffset );

		// Waves
		float waveTime = (float)glfwGetTime() * _waterMaterial.waveSpeed;
		SetUniform( "u_waveTime", waveTime );
		SetUniform( "u_waveLevels", _waterMaterial.waveLevels );
		SetUniform( "u_waveAmplitude", _waterMaterial.waveAmplitude );
		SetUniform( "u_waveFreqBase", _waterMaterial.waveFrquencyBase );
		SetUniform( "u_waveFreqScalar", _waterMaterial.waveFrquencyScalar );
		SetUniform( "u_waveRoughness", _waterMaterial.waveRoughness );
		SetUniform( "u_waveChoppy", _waterMaterial.waveChoppy );
		SetUniform( "u_waveChoppyEase", _waterMaterial.waveChoppyEase );

		// Foam
		SetUniform("u_foamRepeat", _waterMaterial.foamRepeat);
		SetUniform("u_foamStrength", _waterMaterial.foamStrength);
		
		// Dirt
		SetUniform("u_dirtColor", _terrainMaterial.dirtColor);

		// Lighting
		SetUniform("u_lightDir", -glm::euclidean(vec2(_terrainMaterial.lightAltitude, _terrainMaterial.lightAzimuth)));
		SetUniform("u_lightDistance", _terrainMaterial.lightDistance);
		SetUniform("u_lightIntensity", _terrainMaterial.directLightIntensity);
		SetUniform("u_ambientLightIntensity", _terrainMaterial.ambientLightIntensity);

		// Glow
		SetUniform("u_glowScalar", _terrainMaterial.glowScalar);
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	WaterPass::WaterPass(std::string _name)
		: NodeGroupProcess(_name, typeid(WaterPass))
		, RenderPass("WaterPass", godBox::eRenderPhase_Transparent)
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
			auto& terrainMaterial = *node->terrainMaterial;
			auto& waterMaterial = *node->waterMaterial;

			m_shader.VertexShader().BindPerModel(geom, terrainMaterial, waterMaterial);
			m_shader.FragmentShader().BindPerModel(geom, terrainMaterial, waterMaterial);

			geom.Bind();
			geom.Draw();
		}

		glDepthMask(GL_TRUE);
	}
}