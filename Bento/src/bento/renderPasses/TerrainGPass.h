#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupRenderPassBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/geom/TerrainGeometry.h>
#include <bento/components/Transform.h>
#include <bento/materials/TerrainMaterial.h>

namespace bento
{
	struct TerrainGVert
		: ShaderStageBase
	{
		TerrainGVert();
		void BindPerModel(TerrainGeometry* _geometry);
	};

	struct TerrainGFrag
		: ShaderStageBase
	{
		TerrainGFrag();
	};

	struct TerrainGShader
		: ShaderBase<TerrainGVert, TerrainGFrag>
	{
	};

	DEFINE_NODE_3
	(
		TerrainGPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material
	)

	class TerrainGPass
		: public bento::SharedObject<TerrainGPass>
		, public NodeGroupRenderPassBase<TerrainGPassNode>
	{
	public:
		virtual void Render();
		virtual RenderPhase GetRenderPhase() override;

	private:
		TerrainGShader m_shader;
	};
}