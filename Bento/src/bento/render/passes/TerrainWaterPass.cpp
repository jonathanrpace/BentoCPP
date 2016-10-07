#include "TerrainWaterPass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bento/core/Logging.h>

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

	TerrainWaterVert::TerrainWaterVert()
		: ShaderStageBase("shaders/TerrainWater.vert") 
	{
	}

	void TerrainWaterVert::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix());

		SetUniform("u_mapHeightOffset", _material.moltenMapOffset);
		SetUniform("u_waterDepthToOpaque", _material.waterDepthToOpaque);
		SetUniform("u_dissolvedDirtDepthToDiffuse", _material.dissolvedDirtDepthToDiffuse);

		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_velocityData", _geometry.VelocityData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_normalData", _geometry.NormalData().GetRead());
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainWaterFrag::TerrainWaterFrag()
		: ShaderStageBase("shaders/TerrainWater.frag") 
	{
	}

	void TerrainWaterFrag::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		float phase = fmodf( (float)glfwGetTime() * _material.waterFlowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f ) * 2.0f - 1.0f;
		float phaseB = fmodf( phase + 0.5f, 1.0f ) * 2.0f - 1.0f;
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform( "u_phaseA", phaseA );
		SetUniform( "u_phaseB", phaseB );
		SetUniform( "u_phaseAlpha", alphaB );
		SetUniform( "u_waterFlowOffset", _material.waterFlowOffset );
		SetUniform( "u_waterFlowRepeat", _material.waterFlowRepeat );

		float waveTime = (float)glfwGetTime() * _material.waterWaveSpeed;
		SetUniform( "u_waveTime", waveTime );

		SetUniform("u_waterColor", _material.waterColor);
		SetUniform("u_dirtColor", _material.dirtColor);
		SetUniform("u_indexOfRefraction", _material.waterIndexOfRefraction);
		SetUniform("u_specularPower", _material.waterSpecularPower);
		SetUniform("u_waterDepthToDiffuse", _material.waterDepthToDiffuse);
		
		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightDistance", _material.lightDistance);
		SetUniform("u_lightIntensity", _material.directLightIntensity);
		SetUniform("u_ambientLightIntensity", _material.ambientLightIntensity);

		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix(), true);
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix());

		SetTexture("s_output", RenderParams::RenderTarget().OutputTextureA());
		SetTexture("s_positionBuffer", RenderParams::RenderTarget().PositionTexture());
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainWaterPass::TerrainWaterPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainWaterPass))
		, RenderPass(eRenderPhase_Forward)
	{
	}

	void TerrainWaterPass::Advance(double _dt)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
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