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
	struct TerrainVert
		: ShaderStageBase
	{
		TerrainVert();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material);
	};

	struct TerrainFrag
		: ShaderStageBase
	{
		TerrainFrag();
		void BindPerModel(TerrainGeometry& _geometry, TerrainMaterial& _material);
	};

	struct TerrainShader
		: ShaderBase<TerrainVert, TerrainFrag>
	{
	};

	DEFINE_NODE_3
	(
		TerrainPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material
	)

	class TerrainPass
		: public NodeGroupProcess<TerrainPassNode>
		, public RenderPass
		, public SharedObject<TerrainPass>
	{
	public:
		TerrainPass(std::string _name = "TerrainGPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TerrainShader m_shader;
	};
}