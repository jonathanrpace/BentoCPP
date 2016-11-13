#pragma once

// bento
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/Transform.h>
#include <bento/render/shaders/NullFrag.h>

// app
#include <components/materials/TerrainMaterial.h>
#include <components/geom/TerrainGeometry.h>
#include <components/geom/FoamParticleGeom.h>

using namespace bento;

namespace godBox
{
	struct FoamParticleUpdateVert : ShaderStageBase
	{
		FoamParticleUpdateVert();
		virtual void OnPreLink() override;
	};

	struct FoamParticleUpdateShader	: ShaderBase<FoamParticleUpdateVert, NullFrag> {};



	struct FoamVert
		: ShaderStageBase
	{
		FoamVert();
	};

	struct FoamFrag
		: ShaderStageBase
	{
		struct FoamFrag();
	};

	struct FoamShader : ShaderBase<FoamVert, FoamFrag>	{};

	DEFINE_NODE_4
	(
		FoamPassNode,
		TerrainGeometry, geom,
		Transform, transform,
		TerrainMaterial, material,
		FoamParticleGeom, foamGeom
	)

	class FoamPass
		: public NodeGroupProcess<FoamPassNode>
		, public RenderPass
		, public SharedObject<FoamPass>
	{
	public:
		FoamPass(std::string _name = "FoamPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		FoamShader m_shader;
		FoamParticleUpdateShader m_foamParticleUpdateShader;
	};
}