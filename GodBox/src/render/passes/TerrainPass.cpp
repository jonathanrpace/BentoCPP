#include "TerrainPass.h"

#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/polar_coordinates.hpp>

// bento
#include <bento/core/Logging.h>
#include <bento/render/RenderParams.h>

// app
#include <render/eRenderPhase.h>

namespace godBox
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	TerrainVert::TerrainVert() 
		: ShaderStageBase("shaders/Terrain.vert") 
	{
	}

	void TerrainVert::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		// Textures
		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_normalData", _geometry.NormalData().GetRead());
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());

		SetTexture("s_lavaMaterial", _material.lavaMat);

		// Matrices
		SetUniform("u_mvpMatrix", RenderParams::ModelViewProjectionMatrix());
		SetUniform("u_modelViewMatrix", RenderParams::ModelViewMatrix());
		SetUniform("u_viewMatrix", RenderParams::ViewMatrix() );

		// Uniforms
		SetUniform("u_displacement", _material.heightOffset);

		SetUniform("u_cameraPos", RenderParams::CameraPosition());

		SetUniform("u_fogDensity", _material.fogDensity);
		SetUniform("u_fogHeight", _material.fogHeight);
		SetUniform("u_fogColorAway", _material.fogColorAway);
		SetUniform("u_fogColorTowards", _material.fogColorTowards);
		
		SetUniform("u_moltenColor", _material.moltenColor);
		SetUniform("u_moltenColorScalar", _material.moltenColorScalar);
		SetUniform("u_moltenAlphaScalar", _material.moltenAlphaScalar);
		SetUniform("u_moltenAlphaPower", _material.moltenAlphaPower);

		SetUniform("u_dirtHeightToOpaque", _material.dirtHeightToOpaque);
		
		SetUniform("u_heightOffset", _material.heightOffset);
		SetUniform("u_uvRepeat", _material.uvRepeat);

		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightDistance", _material.lightDistance);
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	TerrainFrag::TerrainFrag() 
		: ShaderStageBase("shaders/Terrain.frag") 
	{

	}

	void TerrainFrag::BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material)
	{
		//PRINTF("viewPosition %2f, %2f, %2f\n", RenderParams::CameraPosition().x, RenderParams::CameraPosition().y, RenderParams::CameraPosition().z);

		float phase = fmodf( (float)glfwGetTime() * _material.moltenFlowSpeed, 1.0f );
		float phaseA = fmodf( phase + 0.0f, 1.0f );
		float phaseB = fmodf( phase + 0.5f, 1.0f );
		float alphaB = fabs( 0.5f - phase ) * 2.0f;
		float alphaA = 1.0f - alphaB;

		SetUniform( "u_phaseA", phaseA );
		SetUniform( "u_phaseB", phaseB );
		SetUniform( "u_phaseAlpha", alphaB );
		SetUniform( "u_flowOffset", _material.moltenFlowOffset );

		// Uniforms
		SetUniform("u_cameraPos", RenderParams::CameraPosition());

		SetUniform("u_rockReflectivity", _material.rockReflectivity);
		SetUniform("u_rockDetailBumpStrength", _material.rockDetailBumpStrength);
		SetUniform("u_rockDetailBumpSlopePower", _material.rockDetailBumpSlopePower);

		SetUniform("u_glowScalar", _material.glowScalar);
		SetUniform("u_moltenColor", _material.moltenColor);
		SetUniform("u_moltenColorScalar", _material.moltenColorScalar);

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
		SetTexture("s_smudgeData", _geometry.SmudgeData().GetRead());
		SetTexture("s_velocityData", _geometry.VelocityData().GetRead());
		SetTexture("s_miscData", _geometry.MiscData().GetRead());
		SetTexture("s_heightData", _geometry.HeightData().GetRead());
		SetTexture("s_uvOffsetData", _geometry.UVOffsetData().GetRead());
		
		SetTexture("s_lavaAlbedo", _material.lavaAlb);
		SetTexture("s_lavaNormal", _material.lavaNrm);
		SetTexture("s_lavaMaterial", _material.lavaMat);

		SetTexture("s_lavaLongAlbedo", _material.lavaAlb);
		SetTexture("s_lavaLongNormal", _material.lavaNrm);
		SetTexture("s_lavaLongMaterial", _material.lavaMat);

		SetTexture("s_lavaLatAlbedo", _material.lavaAlb);
		SetTexture("s_lavaLatNormal", _material.lavaNrm);
		SetTexture("s_lavaLatMaterial", _material.lavaMat);

		SetTexture("s_envMap", _material.envMap);
		SetTexture("s_irrMap", _material.irrMap);

		SetTexture("s_moltenGradient", _material.moltenGradient);

		TerrainMousePos terrainMousePos = _geometry.GetTerrainMousePos();
		terrainMousePos.z = INT_MAX;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(terrainMousePos), &terrainMousePos, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _geometry.MousePositionBuffer());
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	TerrainPass::TerrainPass(std::string _name)
		: NodeGroupProcess(_name, typeid(TerrainPass))
		, RenderPass(eRenderPhase_Forward)
	{
	}

	void TerrainPass::Advance(double _dt)
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