#pragma once

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/geom/TerrainGeometry.h>
#include <bento/geom/FoamParticleGeom.h>
#include <bento/components/Transform.h>
#include <bento/materials/TerrainMaterial.h>

namespace bento
{
	struct TerrainFoamVert
		: ShaderStageBase
	{
		TerrainFoamVert();
		//void BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material, vec4 _phase);
	};

	struct TerrainFoamFrag
		: ShaderStageBase
	{
		struct TerrainFoamFrag();
		//void BindPerModel(TerrainGeometry* _geometry, TerrainMaterial* _material, vec4 _phase);
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