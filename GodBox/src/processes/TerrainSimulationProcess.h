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
	struct UpdateTerrainFluxFrag : ShaderStageBase
	{
		UpdateTerrainFluxFrag();
	};

	struct UpdateTerrainDataFrag : ShaderStageBase
	{
		UpdateTerrainDataFrag();
	};

	struct AdvectFrag : ShaderStageBase
	{
		AdvectFrag();
	};

	struct JacobiFrag : ShaderStageBase
	{
		JacobiFrag();
	};

	struct ComputeDivergenceFrag : ShaderStageBase
	{
		ComputeDivergenceFrag();
	};

	struct SubtractGradientFrag : ShaderStageBase
	{
		SubtractGradientFrag();
	};

	struct UpdateTerrainFluxShader		: ShaderBase<ScreenQuadVert, UpdateTerrainFluxFrag> {};
	struct UpdateTerrainDataShader		: ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
	struct AdvectShader					: ShaderBase<ScreenQuadVert, AdvectFrag> {};
	struct JacobiShader					: ShaderBase<ScreenQuadVert, JacobiFrag> {};
	struct ComputeDivergenceShader		: ShaderBase<ScreenQuadVert, ComputeDivergenceFrag> {};
	struct SubtractGradientShader		: ShaderBase<ScreenQuadVert, SubtractGradientFrag> {};
	
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
		void Jacobi(RenderTargetBase& renderTarget, TextureSquare& pressure, TextureSquare& divergence, vec2 cellSize, TextureSquare& dest);
		void SubtractGradient(RenderTargetBase& renderTarget, TextureSquare& velocity, TextureSquare& pressure, TextureSquare& dest);
		
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeRemoved, const TerrainSimPassNode&, node);

		// Shaders
		ScreenQuadGeometry m_screenQuadGeom;
		UpdateTerrainFluxShader m_updateFluxShader;
		UpdateTerrainDataShader m_updateDataShader;
		AdvectShader m_advectShader;
		JacobiShader m_jacobiShader;
		ComputeDivergenceShader m_computeDivergenceShader;
		SubtractGradientShader m_subtractGradientShader;

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
		float m_moltenViscosity;
		float m_rockMeltingPoint;
		float m_heatAdvectSpeed;
		float m_meltCondenseSpeed;
		float m_tempChangeSpeed;
		float m_moltenVelocityScalar;
		float m_smudgeChangeRate;
		float m_moltenSlopeStrength;
		float m_moltenDiffusionStrength;
		float m_moltenVelocityDamping;
		float m_moltenPressureStrength;

		// Dirt
		float m_dirtViscosity;
		float m_dirtMaxSlope;
		float m_dirtDensity;

		// Water
		float m_waterFluxDamping;
		float m_waterViscosity;
		float m_waterBoilingPoint;
		float m_waterFreezingPoint;
		float m_evapourationRate;
		float m_rainRate;
		float m_waterVelocityScalar;
		float m_waterVelocityDamping;
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