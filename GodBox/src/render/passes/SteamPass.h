#pragma once

// bento
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/Transform.h>
#include <bento/render/shaders/NullFrag.h>
#include <bento/core/IInspectable.h>
#include <bento/core/SerializableBase.h>

// app
#include <components/materials/TerrainMaterial.h>
#include <components/materials/SmokeParticleMaterial.h>
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

	DEFINE_NODE_5
	(
		TerrainSteamPassNode,
		TerrainGeometry, terrainGeom,
		Transform, transform,
		TerrainMaterial, terrainMaterial,
		SmokeParticleMaterial, smokeMaterial,
		SteamParticleGeom, particleGeom
	)

	class SteamPass
		: public NodeGroupProcess<TerrainSteamPassNode>
		, public RenderPass
		, public SharedObject<SteamPass>
		, public SerializableBase
		, public IInspectable
	{
	public:
		SteamPass(std::string _name = "SteamPass");

		// From Process
		virtual void Advance(double _dt) override;

		// From IInspectable
		virtual void AddUIElements() override;

	private:
		SteamShader m_drawShader;
		SteamParticleUpdateShader m_updateShader;

		vec2 spawnDelay;
		vec2 spawnThreshold;

		vec3 spawnVelocityMin;
		vec3 spawnVelocityMax;

		float spawnSize;

		vec2 life;

		vec3 positionAccelerationMin;
		vec3 positionAccelerationMax;
		vec2 positionDamping;

		vec2 sizeAcceleration;
		vec2 sizeDamping;

		float fadeInTime;
		float fadeOutTime;
	};
}