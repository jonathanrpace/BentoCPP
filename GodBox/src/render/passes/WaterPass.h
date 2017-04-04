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
#include <components/materials/WaterMaterial.h>
#include <components/geom/TerrainGeometry.h>

using namespace bento;

namespace godBox
{
	struct WaterVert
		: ShaderStageBase
	{
		WaterVert();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _terrainMaterial, WaterMaterial& _waterMaterial);
	};

	struct WaterFrag
		: ShaderStageBase
	{
		WaterFrag();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _terrainMaterial, WaterMaterial& _waterMaterial);
	};

	struct WaterShader
		: ShaderBase<WaterVert, WaterFrag>
	{
	};

	DEFINE_NODE_4
	(
		WaterPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, terrainMaterial,
		WaterMaterial, waterMaterial
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