#pragma once

#include <map>

// bento
#include <bento.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderTargetBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/SerializableBase.h>
#include <bento/render/shaders/ScreenQuadVert.h>
#include <bento/render/shaders/NullFrag.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/Transform.h>

// app
#include <components/materials/TerrainMaterial.h>
#include <components/geom/TerrainGeometry.h>
#include <components/geom/MoltenParticleGeom.h>

using namespace bento;

namespace godBox
{
	struct UpdateFluxFrag : ShaderStageBase
	{
		UpdateFluxFrag();
	};

	struct ApplyFluxFrag : ShaderStageBase
	{
		ApplyFluxFrag();
	};

	struct UpdateTerrainDataFrag : ShaderStageBase
	{
		UpdateTerrainDataFrag();
	};
	
	struct DeriveTerrainDataFrag : ShaderStageBase
	{
		DeriveTerrainDataFrag();
	};

	struct JacobiFrag : ShaderStageBase
	{
		JacobiFrag();
	};

	struct ComputeDivergenceFrag : ShaderStageBase
	{
		ComputeDivergenceFrag();
	};

	struct UpdateFluxShader				: ShaderBase<ScreenQuadVert, UpdateFluxFrag> {};
	struct ApplyFluxShader				: ShaderBase<ScreenQuadVert, ApplyFluxFrag> {};
	struct UpdateTerrainDataShader		: ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
	struct DeriveTerrainDataShader		: ShaderBase<ScreenQuadVert, DeriveTerrainDataFrag> {};
	struct JacobiShader					: ShaderBase<ScreenQuadVert, JacobiFrag> {};
	struct ComputeDivergenceShader		: ShaderBase<ScreenQuadVert, ComputeDivergenceFrag> {};
	
	DEFINE_NODE_3
	(
		TerrainSimPassNode,
		TerrainGeometry, geom,
		TerrainMaterial, material,
		MoltenParticleGeom, moltenParticleGeom
	)

	struct TerrainSimulationProcess
		: NodeGroupProcess<TerrainSimPassNode>
		, SharedObject<TerrainSimulationProcess>
		, IInspectable
		, SerializableBase
	{
		TerrainSimulationProcess(std::string _name = "TerrainSimulationProcess");
		~TerrainSimulationProcess();

		// From Process
		virtual void Advance(double _dt) override;

		// From IInspectable
		virtual void AddUIElements() override;

	private:
		void AdvanceTerrainSim(TerrainGeometry& _geom, TerrainMaterial& _material, RenderTargetBase& _renderTarget, MoltenParticleGeom & _moltenParticleGeom);

		void ClearSurface(RenderTargetBase & renderTarget,TextureSquare & dest,float v);
		

		void ApplyFlux			(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material);
		void UpdateFlux			(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material);
		void UpdateTerrainData	(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material);
		void DeriveTerrainData	(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material);
		void UpdatePressure		(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material);
		void Jacobi				(RenderTargetBase& _renderTarget, TextureSquare& _pressure, TextureSquare& _divergence, vec2 _cellSize, TextureSquare& _dest);
		
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeRemoved, const TerrainSimPassNode&, node);

		// Shaders
		ScreenQuadGeometry m_screenQuadGeom;
		UpdateFluxShader m_updateFluxShader;
		ApplyFluxShader m_applyFluxShader;
		UpdateTerrainDataShader m_updateDataShader;
		DeriveTerrainDataShader m_deriveDataShader;
		JacobiShader m_jacobiShader;
		ComputeDivergenceShader m_computeDivergenceShader;

		// Internal
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;
		float m_prevPhaseA = 0.0f;
		float m_prevPhaseB = 0.0f;

		// Inspectable/Serializable

		// Input
		float m_mouseRadius;
		float m_mouseVolumeStrength;
		float m_mouseHeatStrength;

		// Molten
		float m_moltenViscosityMin;
		float m_moltenViscosityMax;
		float m_moltenSlopeStrength;
		float m_moltenDiffusionStrength;
		float m_moltenPressureStrength;
		float m_meltSpeed;
		float m_condenseSpeed;
		float m_tempChangeSpeed;
		float m_smudgeChangeRate;
		
		// Dirt
		float m_dirtViscosity;
		float m_dirtMaxSlope;
		float m_dirtDensity;

		// Water
		float m_waterViscosity;
		float m_waterSlopeStrength;
		float m_waterBoilingPoint;
		float m_waterFreezingPoint;
		float m_evapourationRate;
		float m_rainRate;
		float m_boilSpeed;
		float m_drainRate;
		float m_drainMaxDepth;

		// Foam
		float m_foamSpawnStrength;
		float m_foamDecayRate;

		// Erosion
		float m_erosionStrength;
		float m_erosionWaterSpeedMax;
		float m_erosionWaterDepthMin;
		float m_erosionWaterDepthMax;
		float m_erosionDirtDepthMax;

		// Dirt transport
		float m_dirtTransportSpeed;
		float m_dirtPickupRate;
		float m_dirtPickupMinWaterSpeed;
		float m_dirtDepositSpeed;
		float m_dissolvedDirtSmoothing;

		// Global
		float m_ambientTemperature;
		float m_timeStep;
	};
}