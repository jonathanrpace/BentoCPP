#pragma once

#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/geom/TerrainGeometry.h>
#include <bento/components/geom/SteamParticleGeom.h>
#include <bento/components/Transform.h>
#include <bento/components/materials/TerrainMaterial.h>
#include <bento/render/shaders/NullFrag.h>

namespace bento
{
	struct SteamParticleUpdateVert : ShaderStageBase
	{
		SteamParticleUpdateVert();
		virtual void OnPreLink() override;
	};

	struct SteamParticleUpdateShader	: ShaderBase<SteamParticleUpdateVert, NullFrag> {};

	struct TerrainSteamVert
		: ShaderStageBase
	{
		TerrainSteamVert();
	};

	struct TerrainSteamFrag
		: ShaderStageBase
	{
		struct TerrainSteamFrag();
	};

	struct TerrainSteamShader : ShaderBase<TerrainSteamVert, TerrainSteamFrag>	{};

	DEFINE_NODE_4
	(
		TerrainSteamPassNode,
		TerrainGeometry, terrainGeom,
		Transform, transform,
		TerrainMaterial, material,
		SteamParticleGeom, particleGeom
	)

	class TerrainSteamPass
		: public NodeGroupProcess<TerrainSteamPassNode>
		, public RenderPass
		, public SharedObject<TerrainSteamPass>
	{
	public:
		TerrainSteamPass(std::string _name = "TerrainSteamPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		TerrainSteamShader m_drawShader;
		SteamParticleUpdateShader m_updateShader;
	};
}