#pragma once

#include <map>

#include <bento.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/RenderTargetBase.h>
#include <bento/render/shaders/ScreenQuadVert.h>
#include <bento/render/shaders/NullFrag.h>
#include <bento/components/geom/TerrainGeometry.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/geom/FoamParticleGeom.h>
#include <bento/components/materials/TerrainMaterial.h>
#include <bento/components/Transform.h>
#include <bento/core/SerializableBase.h>

namespace bento
{
	struct UpdateTerrainFluxFrag : ShaderStageBase
	{
		UpdateTerrainFluxFrag();
	};

	struct UpdateTerrainFluxOneAxisFrag : ShaderStageBase
	{
		UpdateTerrainFluxOneAxisFrag();
	};

	struct UpdateTerrainDataFrag : ShaderStageBase
	{
		UpdateTerrainDataFrag();
	};

	struct UpdateTerrainHeightsFrag : ShaderStageBase
	{
		UpdateTerrainHeightsFrag();
	};

	struct UpdateTerrainMiscFrag : ShaderStageBase
	{
		UpdateTerrainMiscFrag();
	};

	struct FoamParticleUpdateVert : ShaderStageBase
	{
		FoamParticleUpdateVert();
		virtual void OnPreLink() override;
	};

	struct FoamVert : ShaderStageBase
	{
		FoamVert();
	};

	struct FoamFrag : ShaderStageBase
	{
		struct FoamFrag();
	};

	struct DiffuseHeightFrag : ShaderStageBase
	{
		DiffuseHeightFrag();
	};

	struct UpdateTerrainFluxShader			: ShaderBase<ScreenQuadVert, UpdateTerrainFluxFrag> {};
	struct UpdateTerrainFluxOneAxisShader	: ShaderBase<ScreenQuadVert, UpdateTerrainFluxOneAxisFrag> {};
	struct UpdateTerrainDataShader			: ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
	struct UpdateTerrainHeightsShader		: ShaderBase<ScreenQuadVert, UpdateTerrainHeightsFrag> {};
	struct UpdateTerrainMiscShader			: ShaderBase<ScreenQuadVert, UpdateTerrainMiscFrag> {};
	struct FoamParticleUpdateShader			: ShaderBase<FoamParticleUpdateVert, NullFrag> {};
	struct DiffuseHeightShader				: ShaderBase<ScreenQuadVert, DiffuseHeightFrag> {};
	struct FoamShader						: ShaderBase<FoamVert, FoamFrag> {};
	
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
		, SerializableBase
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

		ScreenQuadGeometry m_screenQuadGeom;
		UpdateTerrainFluxShader m_updateFluxShader;
		UpdateTerrainFluxOneAxisShader m_updateFluxOneAxisShader;
		UpdateTerrainDataShader m_updateDataShader;
		UpdateTerrainHeightsShader m_updateHeightsShader;
		UpdateTerrainMiscShader m_updateMiscShader;
		FoamParticleUpdateShader m_foamParticleUpdateShader;
		FoamShader m_foamShader;
		DiffuseHeightShader m_diffuseHeightShader;
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;

		// Input
		float m_mouseRadius = 0.1f;
		float m_mouseVolumeStrength = 0.002f;
		float m_mouseHeatStrength = 0.08f;

		// Molten
		float m_moltenFluxDamping = 0.99f;
		float m_moltenViscosityMin = 0.05f;
		float m_moltenViscosityMax;
		float m_rockMeltingPoint;
		float m_textureScrollSpeed;
		float m_textureCycleSpeed;
		float m_heatAdvectSpeed;
		float m_meltSpeed;
		float m_condenseSpeed;
		float m_tempChangeSpeed;

		// Water
		float m_waterFluxDamping;
		float m_waterViscosity;
		float m_waterBoilingPoint;
		float m_waterFreezingPoint;
		float m_evapourationRate;
		float m_rainRate;
		float m_waterVelocityScalar;
		float m_waterVelocityDamping;

		// Erosion
		float m_erosionStrength;
		float m_erosionWaterSpeedMax;
		float m_erosionWaterDepthMin;
		float m_erosionWaterDepthMax;
		float m_erosionDirtDepthMax;

		float m_dirtErodeSpeedMax;

		float m_dirtTransportSpeed;
		float m_dirtPickupSpeed;
		float m_dirtDepositSpeed;
		float m_dirtDiffuseStrength;
		float m_waterDiffuseStrength;
		

		// Global
		float m_ambientTemperature;
	};
}