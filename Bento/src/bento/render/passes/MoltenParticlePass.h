#pragma once

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/geom/TerrainGeometry.h>
#include <bento/components/geom/MoltenParticleGeom.h>
#include <bento/components/Transform.h>
#include <bento/components/materials/TerrainMaterial.h>

namespace bento
{
	struct MoltenParticleVert
		: ShaderStageBase
	{
		MoltenParticleVert();
	};

	struct MoltenParticleFrag
		: ShaderStageBase
	{
		struct MoltenParticleFrag();
	};

	struct MoltenParticleShader
		: ShaderBase<MoltenParticleVert, MoltenParticleFrag>
	{
	};

	DEFINE_NODE_4
	(
		MoltenParticlePassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material,
		MoltenParticleGeom, moltenParticleGeom
	)

	class MoltenParticlePass
		: public NodeGroupProcess<MoltenParticlePassNode>
		, public RenderPass
		, public SharedObject<MoltenParticlePass>
	{
	public:
		MoltenParticlePass(std::string _name = "MoltenParticlePass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		MoltenParticleShader m_shader;
	};
}