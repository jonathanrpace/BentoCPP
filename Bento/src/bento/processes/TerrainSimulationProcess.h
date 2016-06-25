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
#include <bento/components/geom/MoltenParticleGeom.h>
#include <bento/components/materials/TerrainMaterial.h>
#include <bento/components/Transform.h>
#include <bento/core/SerializableBase.h>

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

	struct DiffuseHeightFrag : ShaderStageBase
	{
		DiffuseHeightFrag();
	};

	struct MoltenParticleUpdateVert : ShaderStageBase
	{
		MoltenParticleUpdateVert();
		virtual void OnPreLink() override;
	};
	
	struct MoltenMapVert : ShaderStageBase
	{
		MoltenMapVert();
	};

	struct MoltenMapFrag : ShaderStageBase
	{
		struct MoltenMapFrag();
	};
	
	struct UpdateTerrainFluxShader			: ShaderBase<ScreenQuadVert, UpdateTerrainFluxFrag> {};
	struct UpdateTerrainDataShader			: ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
	
	struct MoltenParticleUpdateShader		: ShaderBase<MoltenParticleUpdateVert, NullFrag> {};
	struct DiffuseHeightShader				: ShaderBase<ScreenQuadVert, DiffuseHeightFrag> {};
	struct MoltenMapShader					: ShaderBase<MoltenMapVert, MoltenMapFrag> {};
	
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
		TerrainSimulationProcess(std::string _name = "TerrainSimulationPass");
		~TerrainSimulationProcess();

		// From Process
		virtual void Advance(double _dt) override;

		// From IInspectable
		virtual void AddUIElements() override;

	private:
		void AdvanceTerrainSim(TerrainGeometry& _geom, TerrainMaterial& _material, RenderTargetBase& _renderTarget, MoltenParticleGeom & _moltenParticleGeom);

		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationProcess, OnNodeRemoved, const TerrainSimPassNode&, node);

		ScreenQuadGeometry m_screenQuadGeom;
		UpdateTerrainFluxShader m_updateFluxShader;
		UpdateTerrainDataShader m_updateDataShader;
		MoltenParticleUpdateShader m_moltenParticleUpdateShader;
		MoltenMapShader m_moltenMapShader;
		DiffuseHeightShader m_diffuseHeightShader;
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;

		// Input
		float m_mouseRadius;
		float m_mouseVolumeStrength;
		float m_mouseHeatStrength;

		// Molten
		float m_moltenFluxDamping;
		float m_moltenViscosity;
		float m_rockMeltingPoint;
		float m_textureScrollSpeed;
		float m_textureCycleSpeed;
		float m_heatAdvectSpeed;
		float m_heatDiffuseStrength;
		float m_meltSpeed;
		float m_condenseSpeed;
		float m_tempChangeSpeed;
		float m_moltenVelocityScalar;
		float m_moltenVelocityDamping;

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

		// Erosion
		float m_erosionStrength;
		float m_erosionWaterSpeedMax;
		float m_erosionWaterDepthMin;
		float m_erosionWaterDepthMax;
		float m_erosionDirtDepthMax;

		float m_dirtTransportSpeed;
		float m_dirtPickupRate;
		float m_dirtPickupMinWaterSpeed;
		float m_dirtDepositSpeed;
		float m_dirtDiffuseStrength;
		float m_waterDiffuseStrength;

		// Vegetation
		float m_vegMinDirt;
		float m_vegMaxDirt;
		float m_vegGrowthRate;
		float m_vegMinSlope;
		float m_vegMaxSlope;

		// Global
		float m_ambientTemperature;
	};
}