#pragma once

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/geom/TerrainGeometry.h>
#include <bento/components/Transform.h>
#include <bento/materials/TerrainMaterial.h>

namespace bento
{
	struct TerrainWaterVert
		: ShaderStageBase
	{
		TerrainWaterVert();
		void BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material);
	};

	struct TerrainWaterFrag
		: ShaderStageBase
	{
		TerrainWaterFrag();
		void BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material);
	};

	struct TerrainWaterShader
		: ShaderBase<TerrainWaterVert, TerrainWaterFrag>
	{
	};

	DEFINE_NODE_3
	(
		TerrainWaterPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material
	)

	class TerrainWaterPass
		: public NodeGroupProcess<TerrainWaterPassNode>
		, public RenderPass
		, public SharedObject<TerrainWaterPass>
	{
	public:
		TerrainWaterPass(std::string _name = "TerrainWaterPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TerrainWaterShader m_shader;
	};
}