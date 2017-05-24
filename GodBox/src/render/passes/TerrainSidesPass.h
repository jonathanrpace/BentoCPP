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
#include <components/geom/TerrainSidesGeometry.h>
#include <components/geom/TerrainGeometry.h>

using namespace bento;

namespace godBox
{
	struct TerrainSidesVert
		: ShaderStageBase
	{
		TerrainSidesVert();
		void BindPerModel(TerrainGeometry&, TerrainSidesGeometry&, TerrainMaterial&);
	};

	struct TerrainSidesFrag
		: ShaderStageBase
	{
		TerrainSidesFrag();
		void BindPerModel(TerrainGeometry&, TerrainSidesGeometry&, TerrainMaterial&);
	};

	struct TerrainSidesShader
		: ShaderBase<TerrainSidesVert, TerrainSidesFrag>
	{
	};

	DEFINE_NODE_4
	(
		TerrainSidesPassNode,
		TerrainGeometry, geom,
		TerrainSidesGeometry, sidesGeom,
		Transform, transform,
		TerrainMaterial, material
	)

	class TerrainSidesPass
		: public NodeGroupProcess<TerrainSidesPassNode>
		, public RenderPass
		, public SharedObject<TerrainSidesPass>
	{
	public:
		TerrainSidesPass(std::string _name = "TerrainSidesPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TerrainSidesShader m_shader;
	};
}