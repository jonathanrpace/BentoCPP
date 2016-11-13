#pragma once

// bento
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/Transform.h>

// app
#include <components/materials/TerrainMaterial.h>
#include <components/geom/TerrainGeometry.h>

using namespace bento;

namespace godBox
{
	struct WaterVert
		: ShaderStageBase
	{
		WaterVert();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material);
	};

	struct WaterFrag
		: ShaderStageBase
	{
		WaterFrag();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material);
	};

	struct WaterShader
		: ShaderBase<WaterVert, WaterFrag>
	{
	};

	DEFINE_NODE_3
	(
		WaterPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material
	)

	class WaterPass
		: public NodeGroupProcess<WaterPassNode>
		, public RenderPass
		, public SharedObject<WaterPass>
	{
	public:
		WaterPass(std::string _name = "WaterPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		WaterShader m_shader;
	};
}