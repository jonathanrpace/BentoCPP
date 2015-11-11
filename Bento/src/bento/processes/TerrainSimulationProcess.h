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

		float m_viscosity = 0.1f;
		float m_elasticity = 0.25f;
		float m_mouseRadius = 0.1f;
		float m_mouseStrength = 0.001f;
		float m_textureScrollSpeed = 0.75f;
		float m_smoothingStrength = 0.1f;
	};
}