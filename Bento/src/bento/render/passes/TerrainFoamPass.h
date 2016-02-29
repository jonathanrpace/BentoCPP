#pragma once

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/geom/TerrainGeometry.h>
#include <bento/components/geom/FoamParticleGeom.h>
#include <bento/components/Transform.h>
#include <bento/components/materials/TerrainMaterial.h>

namespace bento
{
	struct TerrainFoamVert
		: ShaderStageBase
	{
		TerrainFoamVert();
	};

	struct TerrainFoamFrag
		: ShaderStageBase
	{
		struct TerrainFoamFrag();
	};

	struct TerrainFoamShader
		: ShaderBase<TerrainFoamVert, TerrainFoamFrag>
	{
	};

	DEFINE_NODE_4
	(
		TerrainFoamPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material,
		FoamParticleGeom, foamGeom
	)

	class TerrainFoamPass
		: public NodeGroupProcess<TerrainFoamPassNode>
		, public RenderPass
		, public SharedObject<TerrainFoamPass>
	{
	public:
		TerrainFoamPass(std::string _name = "TerrainFoamPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TerrainFoamShader m_shader;
	};
}