#include "TerrainGPass.h"

#include <glm/glm.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include <bento/core/Logging.h>
#include <bento/render/RenderParams.h>

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
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());

		SetTexture("s_heightData", &(_geometry->HeightDataRead()));
		SetTexture("s_velocityData", &(_geometry->RockFluxDataRead()));
		SetTexture("s_miscData", &(_geometry->MiscDataRead()));
		SetTexture("s_normalData", &(_geometry->NormalDataRead()));
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

		SetTexture("s_diffuseMap", &(_material->someTexture));
		SetUniform("u_numCells", ivec2(_geometry->NumVerticesPerDimension()));

		SetUniform("u_fogDensity", _material->fogDensity);
		SetUniform("u_fogHeight", _material->fogHeight);
		SetUniform("u_fogFalloff", _material->fogFalloff);
		SetUniform("u_fogColorAway", _material->fogColorAway);
		SetUniform("u_fogColorTowards", _material->fogColorTowards);

		SetUniform("u_cameraPos", RenderParams::CameraPosition());

		//PRINTF("viewPosition %2f, %2f, %2f\n", RenderParams::CameraPosition().x, RenderParams::CameraPosition().y, RenderParams::CameraPosition().z);

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
			//PRINTF("mouseScreenPos %2f, %2f\n", mouseScreenPos.x, mouseScreenPos.y);
			m_shader.FragmentShader().SetUniform("u_mouseScreenPos", mouseScreenPos);

			m_shader.FragmentShader().SetUniform("u_windowSize", m_scene->GetWindow()->GetWindowSize());

			m_shader.FragmentShader().SetUniform("u_viewMatrix", RenderParams::ViewMatrix() );


			node->geom->Draw();
		}
	}
}