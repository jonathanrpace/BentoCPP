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

	struct UpdateTerrainDataFrag : ShaderStageBase
	{
		UpdateTerrainDataFrag();
	};

	struct DiffuseHeightFrag : ShaderStageBase
	{
		DiffuseHeightFrag();
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

	struct UpdateTerrainFluxShader			: ShaderBase<ScreenQuadVert, UpdateTerrainFluxFrag> {};
	struct UpdateTerrainDataShader			: ShaderBase<ScreenQuadVert, UpdateTerrainDataFrag> {};
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
		UpdateTerrainDataShader m_updateDataShader;
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
		float m_moltenViscosity = 0.05f;
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