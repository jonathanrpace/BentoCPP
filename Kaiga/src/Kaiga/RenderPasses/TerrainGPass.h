#pragma once

#include <Ramen/Core/SharedObject.h>

#include <Kaiga/Core/NodeGroupRenderPassBase.h>
#include <Kaiga/Core/ShaderBase.h>
#include <Kaiga/Core/ShaderStageBase.h>
#include <Kaiga/Geometry/TerrainGeometry.h>
#include <Kaiga/Components/Transform.h>
#include <Kaiga/Materials/TerrainMaterial.h>

namespace Kaiga
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
		: public Ramen::SharedObject<TerrainGPass>
		, public NodeGroupRenderPassBase<TerrainGPassNode>
	{
	public:
		virtual void Render();
		virtual RenderPhase GetRenderPhase() override;

	private:
		TerrainGShader m_shader;
	};
}