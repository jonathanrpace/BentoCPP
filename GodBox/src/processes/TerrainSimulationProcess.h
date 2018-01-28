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
#include <components/simulation/TerrainSimulation.h>
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
	
	DEFINE_NODE_4
	(
		TerrainSimPassNode,
		TerrainGeometry, geom,
		TerrainMaterial, material,
		TerrainSimulation, terrainSim,
		MoltenParticleGeom, moltenParticleGeom
	)

	struct TerrainSimulationProcess
		: NodeGroupProcess<TerrainSimPassNode>
		, SharedObject<TerrainSimulationProcess>
	{
		TerrainSimulationProcess(std::string _name = "TerrainSimulationProcess");
		~TerrainSimulationProcess();

		// From Process
		virtual void Advance(double _dt) override;
		
	private:
		void AdvanceTerrainSim(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, MoltenParticleGeom & _moltenParticleGeom, TerrainSimulation & _terrainSimulation);

		void ClearSurface(RenderTargetBase & renderTarget,TextureSquare & dest,float v);
		
		void ApplyFlux			(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, TerrainSimulation& _terrainSim);
		void UpdateFlux			(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, TerrainSimulation& _terrainSim);
		void UpdateTerrainData	(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, TerrainSimulation& _terrainSim);
		void DeriveTerrainData	(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, TerrainSimulation& _terrainSim);
		void UpdatePressure		(RenderTargetBase& _renderTarget, TerrainGeometry& _geom, TerrainMaterial& _material, TerrainSimulation& _terrainSim);
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
	};
}