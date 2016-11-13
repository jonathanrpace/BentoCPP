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
#include <components/geom/SteamParticleGeom.h>

using namespace bento;

namespace godBox
{
	struct SteamParticleUpdateVert : ShaderStageBase
	{
		SteamParticleUpdateVert();
		virtual void OnPreLink() override;
	};

	struct SteamParticleUpdateShader	
		: ShaderBase<SteamParticleUpdateVert, NullFrag> 
	{
	};

	struct SteamVert
		: ShaderStageBase
	{
		SteamVert();
	};

	struct SteamFrag
		: ShaderStageBase
	{
		struct SteamFrag();
	};

	struct SteamShader : ShaderBase<SteamVert, SteamFrag>	{};

	DEFINE_NODE_4
	(
		TerrainSteamPassNode,
		TerrainGeometry, terrainGeom,
		Transform, transform,
		TerrainMaterial, material,
		SteamParticleGeom, particleGeom
	)

	class SteamPass
		: public NodeGroupProcess<TerrainSteamPassNode>
		, public RenderPass
		, public SharedObject<SteamPass>
	{
	public:
		SteamPass(std::string _name = "SteamPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		SteamShader m_drawShader;
		SteamParticleUpdateShader m_updateShader;
	};
}