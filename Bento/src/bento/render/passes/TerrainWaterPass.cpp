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

		SetUniform("u_lightDir", -glm::euclidean(vec2(_material.lightAltitude, _material.lightAzimuth)));
		SetUniform("u_lightIntensity", _material.directLightIntensity);
		SetUniform("u_ambientLightIntensity", _material.ambientLightIntensity);

		SetUniform("u_dirtColor", _material.dirtColor);
		SetUniform("u_specularPower", _material.waterSpecularPower);

		SetTexture("s_heightData", _geometry.HeightDataRead());
		SetTexture("s_velocityData", _geometry.VelocityDataRead());
		SetTexture("s_miscData", _geometry.MiscDataRead());
		SetTexture("s_normalData", _geometry.NormalDataRead());
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
		SetUniform("u_waterColor", _material.waterColor);
		SetUniform("u_indexOfRefraction", _material.waterIndexOfRefraction);

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
		//glEnable(GL_POLYGON_OFFSET_FILL);
		//glPolygonOffset( 5.0f, 0.0f );

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

		//glDisable(GL_POLYGON_OFFSET_FILL);
	}
}