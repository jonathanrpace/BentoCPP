#pragma once

#include <map>

#include <event.h>

#include <bento.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/RenderTargetBase.h>
#include <bento/shaderStages/ScreenQuadVert.h>
#include <bento/geom/TerrainGeometry.h>
#include <bento/geom/ScreenQuadGeometry.h>
#include <bento/materials/TerrainMaterial.h>
#include <bento/components/Transform.h>

namespace bento
{
	struct UpdateFluidFluxFrag : ShaderStageBase
	{
		UpdateFluidFluxFrag();
	};

	struct UpdateFluidHeightFrag : ShaderStageBase
	{
		UpdateFluidHeightFrag();
	};

	struct UpdateFluidVelocityFrag : ShaderStageBase
	{
		UpdateFluidVelocityFrag();
	};

	struct DiffuseHeightFrag : ShaderStageBase
	{
		DiffuseHeightFrag();
	};

	struct UpdateFluidFluxShader : ShaderBase<ScreenQuadVert, UpdateFluidFluxFrag> {};
	struct UpdateFluidHeightShader : ShaderBase<ScreenQuadVert, UpdateFluidHeightFrag> {};
	struct UpdateFluidVelocityShader : ShaderBase<ScreenQuadVert, UpdateFluidVelocityFrag> {};
	struct DiffuseHeightShader : ShaderBase<ScreenQuadVert, DiffuseHeightFrag> {};

	DEFINE_NODE_2
	(
		TerrainSimPassNode,
		TerrainGeometry, geom,
		TerrainMaterial, material
	)

	struct TerrainSimulationProcess
		: NodeGroupProcess<TerrainSimPassNode>
		, SharedObject<TerrainSimulationProcess>
		, IInspectable
	{
		TerrainSimulationProcess(std::string _name = "TerrainSimulationPass");
		~TerrainSimulationProcess();

		// From Process
		virtual void Advance(double _dt) override;

		// From IInspectable
		virtual void AddUIElements() override;

	private:
		void AdvanceTerrainSim(TerrainGeometry& _geom, TerrainMaterial& _material, RenderTargetBase& _renderTarget);

		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeRemoved, const TerrainSimPassNode&, node);

		bool m_switch;
		ScreenQuadGeometry m_screenQuadGeom;
		UpdateFluidFluxShader m_updateFluxShader;
		UpdateFluidHeightShader m_updateHeightShader;
		UpdateFluidVelocityShader m_updateVelocityShader;
		DiffuseHeightShader m_diffuseHeightShader;
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;

		// Input
		float m_mouseRadius = 0.1f;
		float m_mouseVolumeStrength = 0.005f;
		float m_mouseHeatStrength = 0.3f;

		// Molten
		float m_elasticity = 0.5f;
		float m_fluxDamping = 0.98f;

		float m_viscosityMin = 0.1f;
		float m_viscosityMax = 0.4f;
		float m_heatViscosityPower = 1.0f;
		float m_heatViscosityBias = 0.2f;

		float m_velocityScalar = 1.0f;
		float m_textureScrollSpeed = 0.1f;
		float m_textureCycleSpeed = 1.0f;
		float m_textureCycleDamping = 0.999f;

		float m_heatAdvectSpeed = 0.49f;
		float m_heatSmoothingStrength = 0.05f;

		float m_meltSpeed = 0.00001f;
		float m_condenseSpeed = 0.01f;
		float m_tempChangeSpeed = 0.001f;

		// Global
		float m_ambientTemperature = 0.01f;
		
		GLenum HEIGHT_DATA_A = GL_COLOR_ATTACHMENT0;
		GLenum HEIGHT_DATA_B = GL_COLOR_ATTACHMENT1;
		GLenum FLUX_DATA_A = GL_COLOR_ATTACHMENT2;
		GLenum FLUX_DATA_B = GL_COLOR_ATTACHMENT3;
		GLenum VELOCITY_DATA = GL_COLOR_ATTACHMENT4;
		GLenum MAPPING_DATA_A = GL_COLOR_ATTACHMENT5;
		GLenum MAPPING_DATA_B = GL_COLOR_ATTACHMENT6;
		GLenum NORMAL_DATA = GL_COLOR_ATTACHMENT7;
	};
}