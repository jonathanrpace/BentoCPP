#include "CloudPass.h"

#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtc/matrix_transform.hpp>

// bento
#include <bento/render/RenderParams.h>

// app
#include <render/eRenderPhase.h>

namespace godBox
{
	////////////////////////////////////////////
	// Vertex shader
	////////////////////////////////////////////

	CloudPass::VertShader::VertShader() : ShaderStageBase("shaders/Clouds.vert") {}

	void CloudPass::VertShader::BindPerModel(CloudMaterial& _material)
	{
		// Matrices
		SetUniform( "u_invViewProjMatrix", RenderParams::InvViewProjetionMatrix() );
	}

	////////////////////////////////////////////
	// Fragment shader
	////////////////////////////////////////////

	CloudPass::FragShader::FragShader() : ShaderStageBase("shaders/Clouds.frag") {}

	void CloudPass::FragShader::BindPerModel(CloudMaterial& _material)
	{
		// Transform
		SetUniform( "u_baseScale", _material.baseScale );
		SetUniform( "u_baseScaleVertical", _material.baseScaleVertical );
		SetUniform( "u_detailScale", _material.detailScale );
		SetUniform( "u_detailScaleVertical", _material.detailScaleVertical );
		SetUniform( "u_position", _material.position);
		SetUniform( "u_height", _material.height );
		SetUniform( "u_falloffTop", _material.falloffTop );
		SetUniform( "u_falloffBottom", _material.falloffBottom );

		// Vortex
		SetUniform( "u_numVortexTurns", _material.vortexTurns );

		// Quality
		SetUniform( "u_maxRayLength", _material.maxRayLength );
		SetUniform( "u_coarseStepsPerUnit", _material.coarseStepsPerUnit );
		SetUniform( "u_detailStepsPerUnit", _material.detailStepsPerUnit );
		SetUniform( "u_lightConeMaxLength", _material.lightConeMaxLength );
		
		// Uniforms
		SetUniform( "u_cameraPos", RenderParams::CameraPosition() );
		SetUniform( "u_densityScalar", _material.baseDensityScalar );
		SetUniform( "u_densityOffset", _material.baseDensityOffset );
		SetUniform( "u_absorbtion", _material.absorbtion);
		SetUniform( "u_powderSharpness", _material.powderSharpness);
		SetUniform( "u_scatteringParam", _material.scatteringParam);
		SetUniform( "u_detailMaxDensity", _material.detailMaxDensity);
		SetUniform( "u_detailDensityScalar", _material.detailDensityScalar );
		SetUniform( "u_detailDensityOffset", _material.detailDensityOffset );

		SetUniform("u_lightIntensity", _material.lightIntensity);
		SetUniform("u_lightColor", _material.lightColor);
		SetUniform("u_skyColor", _material.skyColor);

		mat4 coneMatrix;
		coneMatrix = glm::rotate(coneMatrix, _material.lightAltitude, vec3(1.0,0.0,0.0));
		coneMatrix = glm::rotate(coneMatrix, _material.lightAzimuth, vec3(0.0,1.0,0.0));
		SetUniform("u_coneMatrix", mat3(coneMatrix));

		// Textures
		SetTexture( "s_baseShapes", _material.baseShapesTexture3D );
		SetTexture( "s_detailMap", _material.detailTexture3D );
	}

	////////////////////////////////////////////
	// Pass
	////////////////////////////////////////////

	CloudPass::CloudPass(std::string _name)
		: NodeGroupProcess(_name, typeid(CloudPass))
		, RenderPass("CloudPass", eRenderPhase_Forward)
	{
	}

	void CloudPass::Advance(double _dt)
	{
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(false);

		m_shader.BindPerPass();
		for (auto node : m_nodeGroup.Nodes())
		{
			RenderParams::SetModelMatrix(node->transform->matrix);
			
			node->geom->Bind();
			m_shader.VertexShader().BindPerModel(*node->material);
			m_shader.FragmentShader().BindPerModel(*node->material);
			
			node->geom->Draw();
		}

		glDepthMask(true);
		glDisable(GL_BLEND);
	}
}