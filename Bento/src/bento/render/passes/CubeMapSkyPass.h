#pragma once

// bento
#include <bento/core/RenderPass.h>
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/materials/CubeMapSkyMaterial.h>
#include <bento/components/geom/SkyboxGeometry.h>

namespace bento
{
	struct CubeMapSkyVert
		: ShaderStageBase
	{
		CubeMapSkyVert();
	};

	struct CubeMapSkyFrag
		: ShaderStageBase
	{
		CubeMapSkyFrag();
	};

	struct CubeMapSkyShader
		: ShaderBase<CubeMapSkyVert, CubeMapSkyFrag>
	{
	};

	DEFINE_NODE_2
	(
		CubeMapSkyPassNode,
		SkyboxGeometry, geom,
		CubeMapSkyMaterial, material
	)

	class CubeMapSkyPass
		: public RenderPass
		, public NodeGroupProcess<CubeMapSkyPassNode>
		, public SharedObject<CubeMapSkyPass>
	{
	public:
		CubeMapSkyPass(std::string _name="CubeMapSkyPass", int _renderPhase = 0);

		// From Process
		virtual void Advance(double _dt) override;

	private:
		CubeMapSkyShader m_shader;
	};
}