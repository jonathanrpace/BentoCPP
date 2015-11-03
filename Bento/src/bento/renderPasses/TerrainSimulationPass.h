#pragma once

#include <map>

#include <event.h>

#include <bento.h>
#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupRenderPassBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/RenderPhase.h>
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
		void SetDataTextures(TextureSquare* _data0, TextureSquare* _data1, TextureSquare* _data2);
	};

	struct UpdateFluidHeightFrag : ShaderStageBase
	{
		UpdateFluidHeightFrag();
		void SetDataTextures(TextureSquare* _data0, TextureSquare* _data1, TextureSquare* _data2);
	};

	struct UpdateFluidFluxShader : ShaderBase<ScreenQuadVert, UpdateFluidFluxFrag> {};
	struct UpdateFluidHeightShader : ShaderBase<ScreenQuadVert, UpdateFluidHeightFrag> {};

	DEFINE_NODE_2
	(
		TerrainSimPassNode,
		TerrainGeometry, geom,
		TerrainMaterial, material
	)

	struct TerrainSimulationPass
		: NodeGroupRenderPassBase<TerrainSimPassNode>
		, SharedObject<TerrainSimulationPass>
	{
		TerrainSimulationPass(std::string _name = "TerrainSimulationPass");
		~TerrainSimulationPass();

		// From IRenderPass
		virtual void Render();
		virtual RenderPhase GetRenderPhase() override;

	private:
		ScreenQuadGeometry m_screenQuadGeom;
		UpdateFluidFluxShader m_updateFluxShader;
		UpdateFluidHeightShader m_updateHeightShader;
		std::map<const TerrainSimPassNode*, RenderTargetBase*> m_renderTargetByNodeMap;

		void AdvanceTerrainSim(TerrainGeometry& _geom, TerrainMaterial& _material, RenderTargetBase& _renderTarget);

		DEFINE_EVENT_HANDLER_1(TerrainSimulationPass, OnNodeAdded, const TerrainSimPassNode&, node);
		DEFINE_EVENT_HANDLER_1(TerrainSimulationPass, OnNodeRemoved, const TerrainSimPassNode&, node);
	};
}