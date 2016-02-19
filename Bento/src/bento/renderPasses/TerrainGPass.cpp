#include "TerrainGPass.h"

#include <glm/glm.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include <bento/core/Logging.h>
#include <bento/core/RenderParams.h>

namespace bento
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainGVert::TerrainGVert() 
		: ShaderStageBase("shaders/TerrainG.vert") 
	{
	}

	void TerrainGVert::BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material)
	{
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		//SetUniform("u_modelMatrix", RenderParams::ModelMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		//SetUniform("u_normalModelViewMatrix", RenderParams::NormalModelViewMatrix());
		SetUniform("u_mapHeightOffset", _material->MapHeightOffset);

		//SetTexture("s_diffuseMap", &(_material->SomeTexture));

		_geometry->SwapRockData();
		_geometry->SwapRockFluxData();
		_geometry->SwapWaterData();
		_geometry->SwapWaterFluxData();

		SetTexture("s_rockData", &(_geometry->RockDataRead()));
		SetTexture("s_rockFluxData", &(_geometry->RockFluxDataRead()));
		SetTexture("s_rockNormalData", &(_geometry->RockNormalData()));

		SetTexture("s_waterData", &(_geometry->WaterDataRead()));
		SetTexture("s_waterFluxData", &(_geometry->WaterFluxDataRead()));
		SetTexture("s_waterNormalData", &(_geometry->WaterNormalData()));

		SetTexture("s_mappingData", &(_geometry->MappingDataRead()));

		SetUniform("u_wave0", vec4(_material->waveStrength0, _material->waveScale0, _material->waveAngle0, (float)_material->waveSpeed0));
		SetUniform("u_wave1", vec4(_material->waveStrength1, _material->waveScale1, _material->waveAngle1, (float)_material->waveSpeed1));
		SetUniform("u_wave2", vec4(_material->waveStrength2, _material->waveScale2, _material->waveAngle2, (float)_material->waveSpeed2));
		SetUniform("u_wave3", vec4(_material->waveStrength3, _material->waveScale3, _material->waveAngle3, (float)_material->waveSpeed3));
		
		_geometry->SwapRockData();
		_geometry->SwapRockFluxData();
		_geometry->SwapWaterData();
		_geometry->SwapWaterFluxData();
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainGFrag::TerrainGFrag() 
		: ShaderStageBase("shaders/TerrainG.frag") 
	{
	}

	void TerrainGFrag::BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material)
	{
		SetUniform("u_lightDir", -glm::euclidean(vec2(_material->lightAltitude, _material->lightAzimuth)));
		SetUniform("u_lightIntensity", _material->directLightIntensity);
		SetUniform("u_ambientLightIntensity", _material->ambientLightIntensity);

		SetTexture("s_mappingData", &(_geometry->MappingDataRead()));
		SetTexture("s_diffuseMap", &(_material->SomeTexture));
		SetUniform("u_numCells", ivec2(_geometry->NumVerticesPerDimension()));

		
		TerrainMousePos terrainMousePos = _geometry->GetTerrainMousePos();
		terrainMousePos.z = INT_MAX;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(terrainMousePos), &terrainMousePos, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geometry->MousePositionBuffer());
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainGPass::TerrainGPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainGPass))
		, RenderPass(eRenderPhase_G)
	{
	}

	void TerrainGPass::Advance(double _dt)
	{
		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			node->geom->Bind();
			m_shader.VertexShader().BindPerModel(node->geom, node->material);
			m_shader.FragmentShader().BindPerModel(node->geom, node->material);
			
			vec2 normalisedMousePos = m_scene->GetInputManager()->GetMousePosition();
			normalisedMousePos /= m_scene->GetWindow()->GetWindowSize();
			normalisedMousePos.y = 1.0f - normalisedMousePos.y;
			vec2 mouseScreenPos = (normalisedMousePos - vec2(0.5f)) * vec2(2.0f);
			PRINTF("mouseScreenPos %2f, %2f\n", mouseScreenPos.x, mouseScreenPos.y);
			m_shader.FragmentShader().SetUniform("u_mouseScreenPos", mouseScreenPos);

			m_shader.FragmentShader().SetUniform("u_windowSize", m_scene->GetWindow()->GetWindowSize());

			m_shader.FragmentShader().SetUniform("u_viewMatrix", RenderParams::ViewMatrix() );


			node->geom->Draw();
		}
	}
}