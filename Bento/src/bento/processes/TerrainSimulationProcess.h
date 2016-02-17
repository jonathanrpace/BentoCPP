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
#include <bento/shaderStages/NullFrag.h>
#include <bento/geom/TerrainGeometry.h>
#include <bento/geom/ScreenQuadGeometry.h>
#include <bento/geom/FoamParticleGeom.h>
#include <bento/materials/TerrainMaterial.h>
#include <bento/components/Transform.h>

namespace bento
{
	struct UpdateTerrainFluxFrag : ShaderStageBase
	{
		UpdateTerrainFluxFrag();
	};

	struct UpdateTerrainDataFrag : ShaderStageBase
	{
		UpdateTerrainDataFrag();
	};

	struct UpdateTerrainMiscFrag : ShaderStageBase
	{
		UpdateTerrainMiscFrag();
	};

	struct UpdateTerrainSmthFrag : ShaderStageBase
	{
		UpdateTerrainSmthFrag();
	};

	struct FoamParticleUpdateVert : ShaderStageBase
	{
		FoamParticleUpdateVert();
		virtual void OnPreLink() override;
	};

	struct UpdateTerrainFluxShader : ShaderBase<ScreenQuadVert, UpdateTerrainFluxFrag> {};
	struct UpdateTerrainDataShader : ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
	struct UpdateTerrainMiscShader : ShaderBase<ScreenQuadVert, UpdateTerrainMiscFrag> {};
	struct UpdateTerrainSmthShader : ShaderBase<ScreenQuadVert, UpdateTerrainSmthFrag> {};
	struct FoamParticleUpdateShader : ShaderBase<FoamParticleUpdateVert, NullFrag> {};

	DEFINE_NODE_3
	(
		TerrainSimPassNode,
		TerrainGeometry, geom,
		TerrainMaterial, material,
		FoamParticleGeom, foamGeom
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
		void AdvanceTerrainSim(TerrainGeometry& _geom, TerrainMaterial& _material, RenderTargetBase& _renderTarget, FoamParticleGeom & _foamParticleGeom);

		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeRemoved, const TerrainSimPassNode&, node);

		bool m_switch;
		ScreenQuadGeometry m_screenQuadGeom;
		UpdateTerrainFluxShader m_updateFluxShader;
		UpdateTerrainDataShader m_updateDataShader;
		UpdateTerrainMiscShader m_updateMiscShader;
		UpdateTerrainSmthShader m_updateSmthShader;
		FoamParticleUpdateShader m_foamParticleUpdateShader;
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;

		// Input
		float m_mouseRadius = 0.1f;
		float m_mouseMoltenVolumeStrength = 0.005f;
		float m_mouseHeatStrength = 0.3f;

		// Molten
		float m_rockFluxDamping = 0.99f;

		float m_viscosityMin = 0.01f;
		float m_viscosityMax = 0.4f;
		float m_heatViscosityPower = 1.5f;
		float m_rockMeltingPoint = 0.3f;

		float m_textureScrollSpeed = 0.04f;
		float m_textureCycleSpeed = 0.05f;

		float m_heatAdvectSpeed = 1.0f;
		float m_heatSmoothingStrength = 0.002f;

		float m_meltSpeed = 0.00001f;
		float m_condenseSpeed = 0.01f;
		float m_tempChangeSpeed = 0.0001f;

		// Water
		float m_waterFluxDamping = 0.99f;

		float m_waterViscosity = 0.5f;
		float m_waterBoilingPoint = 0.1f;
		float m_waterFreezingPoint = 0.0f;

		float m_erosionSpeed = 0.01f;
		float m_erosionFluxMin = 0.01f;
		float m_erosionFluxMax = 0.05f;
		float m_depositionSpeed = 0.01f;

		// Global
		float m_ambientTemperature = 0.05f;
	};
}