#include "TerrainGPass.h"

#include <glfw3.h>

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

	void TerrainGVert::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		// Textures
		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_velocityData", _geometry.VelocityData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_normalData", _geometry.NormalData().GetRead());
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());
		SetTexture("s_diffuseMap", _material.someTexture);
		SetTexture("s_rockDiffuse", _material.rockDiffuseTexture);
		SetTexture("s_moltenMapData", _geometry.MoltenMapData().GetRead());

		// Matrices
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix() );

		// Uniforms
		SetUniform("u_cameraPos", RenderParams::CameraPosition());

		SetUniform("u_fogDensity", _material.fogDensity);
		SetUniform("u_fogHeight", _material.fogHeight);
		SetUniform("u_fogColorAway", _material.fogColorAway);
		SetUniform("u_fogColorTowards", _material.fogColorTowards);
				
		SetUniform("u_rockDetailBumpStrength", _material.rockDetailBumpStrength);
		SetUniform("u_rockDetailDiffuseStrength", _material.rockDetailDiffuseStrength);
		
		SetUniform("u_moltenColor", _material.moltenColor);
		SetUniform("u_moltenColorScalar", _material.moltenColorScalar);
		SetUniform("u_moltenAlphaScalar", _material.moltenAlphaScalar);
		SetUniform("u_moltenAlphaPower", _material.moltenAlphaPower);

		SetUniform("u_dirtHeightToOpaque", _material.dirtHeightToOpaque);
		
		SetUniform("u_mapHeightOffset", _material.moltenMapOffset);

		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightDistance", _material.lightDistance);
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainGFrag::TerrainGFrag() 
		: ShaderStageBase("shaders/TerrainG.frag") 
	{

	}

	void TerrainGFrag::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		//PRINTF("viewPosition %2f, %2f, %2f\n", RenderParams::CameraPosition().x, RenderParams::CameraPosition().y, RenderParams::CameraPosition().z);

		float phase = fmodf( (float)glfwGetTime() * _material.creaseFlowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f ) * 2.0f - 1.0f;
		float phaseB = fmodf( phase + 0.5f, 1.0f ) * 2.0f - 1.0f;
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform( "u_phaseA", phaseA );
		SetUniform( "u_phaseB", phaseB );
		SetUniform( "u_phaseAlpha", alphaB );
		SetUniform( "u_flowOffset", _material.creaseFlowOffset );

		// Uniforms
		SetUniform("u_cameraPos", RenderParams::CameraPosition());

		SetUniform("u_rockColorA", _material.rockColorA);
		SetUniform("u_rockColorB", _material.rockColorB);
		SetUniform("u_rockRoughnessA", _material.rockRoughnessA);
		SetUniform("u_rockRoughnessB", _material.rockRoughnessB);
		SetUniform("u_rockFresnelA", _material.rockFresnelA);
		SetUniform("u_rockFresnelB", _material.rockFresnelB);

		SetUniform("u_hotRockColor", _material.hotRockColor);
		SetUniform("u_hotRockRoughness", _material.hotRockRoughness);
		SetUniform("u_hotRockFresnel", _material.hotRockFresnel);

		SetUniform("u_moltenPlateAlpha", _material.moltenPlateAlpha);
		SetUniform("u_moltenPlateAlphaPower", _material.moltenPlateAlphaPower);
		SetUniform("u_moltenCreaseAlpha", _material.moltenCreaseAlpha);
		SetUniform("u_moltenCreaseAlphaPower", _material.moltenCreaseAlphaPower);

		SetUniform("u_glowScalar", _material.glowScalar);
		SetUniform("u_glowPower", _material.glowPower);
		SetUniform("u_glowDetailScalar", _material.glowDetailScalar);
		SetUniform("u_glowDetailPower", _material.glowDetailPower);



		SetUniform("u_rockDetailDiffuseStrength", _material.rockDetailDiffuseStrength);
		SetUniform("u_rockDetailBumpStrength", _material.rockDetailBumpStrength);

		SetUniform("u_dirtColor", _material.dirtColor);

		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightDistance", _material.lightDistance);
		SetUniform("u_lightIntensity", _material.directLightIntensity);
		SetUniform("u_ambientLightIntensity", _material.ambientLightIntensity);

		// Creases
		SetUniform("u_bearingCreaseScalar", _material.bearingCreaseScalar);
		SetUniform("u_lateralCreaseScalar", _material.lateralCreaseScalar);
		SetUniform("u_creaseRatio", _material.creaseRatio);
		SetUniform("u_creaseMipLevel", _material.creaseMipLevel);
		SetUniform("u_creaseForwardScalar", _material.creaseForwardScalar);
		SetUniform("u_creaseMapRepeat", _material.creaseMapRepeat);
		SetUniform("u_creaseGridRepeat", _material.creaseGridRepeat);

		// Textures
		SetTexture("s_rockDiffuse", _material.rockDiffuseTexture);
		SetTexture("s_creaseMap", _material.creaseTexture);
		SetTexture("s_moltenMapData", _geometry.MoltenMapData().GetRead());
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());
		SetTexture("s_velocityData", _geometry.VelocityData().GetRead());
		
		TerrainMousePos terrainMousePos = _geometry.GetTerrainMousePos();
		terrainMousePos.z = INT_MAX;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(terrainMousePos), &terrainMousePos, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geometry.MousePositionBuffer());
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
			m_shader.VertexShader().BindPerModel(*node->geom, *node->material);
			m_shader.FragmentShader().BindPerModel(*node->geom, *node->material);
			
			// Mouse isect uniforms
			{
				vec2 normalisedMousePos = m_scene->GetInputManager().GetMousePosition();
				normalisedMousePos /= m_scene->GetWindow().GetWindowSize();
				normalisedMousePos.y = 1.0f - normalisedMousePos.y;
				vec2 mouseScreenPos = (normalisedMousePos - vec2(0.5f)) * vec2(2.0f);
				//PRINTF("mouseScreenPos %2f, %2f\n", mouseScreenPos.x, mouseScreenPos.y);
				m_shader.FragmentShader().SetUniform("u_mouseScreenPos", mouseScreenPos);
				m_shader.FragmentShader().SetUniform("u_windowSize", m_scene->GetWindow().GetWindowSize());
			}

			node->geom->Draw();
		}
	}
}