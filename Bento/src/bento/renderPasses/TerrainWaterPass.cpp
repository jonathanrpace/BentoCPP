#include "TerrainWaterPass.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <bento/core/Logging.h>

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

	TerrainWaterVert::TerrainWaterVert()
		: ShaderStageBase("shaders/TerrainWater.vert") 
	{
	}

	void TerrainWaterVert::BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material)
	{
		
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());

		_geometry->SwapRockData();
		_geometry->SwapRockFluxData();
		_geometry->SwapWaterData();
		_geometry->SwapWaterFluxData();

		SetTexture("s_rockData", &(_geometry->RockDataRead()));
		SetTexture("s_waterData", &(_geometry->WaterDataRead()));
		SetTexture("s_waterFluxData", &(_geometry->WaterFluxDataRead()));
		SetTexture("s_waterNormalData", &(_geometry->WaterNormalData()));
		SetTexture("s_rockNormalData", &(_geometry->RockNormalData()));
		SetTexture("s_diffuseMap", &(_material->SomeTexture));
		
		_geometry->SwapRockData();
		_geometry->SwapRockFluxData();
		_geometry->SwapWaterData();
		_geometry->SwapWaterFluxData();
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainWaterFrag::TerrainWaterFrag()
		: ShaderStageBase("shaders/TerrainWater.frag") 
	{
	}

	void TerrainWaterFrag::BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material)
	{
		SetUniform("u_lightDir", -glm::euclidean(vec2(_material->lightAltitude, _material->lightAzimuth)));
		SetUniform("u_lightIntensity", _material->directLightIntensity);
		SetUniform("u_ambientLightIntensity", _material->ambientLightIntensity);

		SetUniform("u_waterColor", _material->waterColor);
		SetUniform("u_waterTranslucentColor", _material->waterTranslucentColor);
		SetUniform("u_specularPower", _material->waterSpecularPower);
		SetUniform("u_indexOfRefraction", _material->waterIndexOfRefraction);

		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix(), true);
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix());

		SetTexture("s_output", RenderParams::DeferedRenderTarget->OutputTextureA());
		SetTexture("s_positionBuffer", RenderParams::DeferedRenderTarget->PositionTexture());

		SetTexture("s_rockData", &_geometry->RockDataRead());
		SetTexture("s_waterData", &_geometry->WaterDataRead());
		SetTexture("s_waterFluxData", &_geometry->WaterFluxDataRead());
		SetTexture("s_mappingData", &_geometry->MappingDataRead());
		SetTexture("s_diffuseMap", &_material->SomeTexture);
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
		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			node->geom->Bind();

			// One cycle of a phase is a minute
			float phase = fmod((float)glfwGetTime() * (1.0f/60.0f), 1.0f);
			/*
			float phaseSpeed = 0.4f;
			float flowSpeed = 2.0f;
			float phase = fmod((float)glfwGetTime() * phaseSpeed, 1.0f);
			float offsetPhaseA = phase;
			float offsetA = offsetPhaseA * flowSpeed;
			float offsetPhaseB = fmod(phase + 0.5f, 1.0f);
			float offsetB = offsetPhaseB * flowSpeed;
			float alpha = phase < 0.5f ? phase / 0.5f : 1.0f - (phase - 0.5f) / 0.5f;
			alpha = 1.0f - alpha;

			vec4 phaseVec = vec4(1.0f - alpha, alpha, offsetA, offsetB);

			
			*/

			m_shader.VertexShader().BindPerModel(node->geom, node->material);
			m_shader.FragmentShader().BindPerModel(node->geom, node->material);

			m_shader.VertexShader().SetUniform("u_phase", phase);

			node->geom->Draw();
		}
	}
}