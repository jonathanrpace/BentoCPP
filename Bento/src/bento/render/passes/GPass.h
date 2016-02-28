#pragma once

#include <string>

#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/RenderPass.h>
#include <bento/components/geom/Geometry.h>
#include <bento/components/Transform.h>
#include <bento/components/materials/StandardMaterial.h>

namespace bento
{
	struct GVert
		: ShaderStageBase
	{
		GVert();
		void BindPerModel();
	};

	struct GFrag
		: ShaderStageBase
	{
		GFrag();
		void BindPerMaterial(StandardMaterial* _material);
	};

	struct GShader
		: ShaderBase<GVert, GFrag>
	{};

	DEFINE_NODE_3
	(
		GPassNode,
		Geometry, geom,
		Transform, transform,
		StandardMaterial, material
	)

	class GPass
		: public NodeGroupProcess<GPassNode>
		, public RenderPass
		, public SharedObject<GPass>
	{
	public:

		GPass(std::string _name = "GPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		GShader m_shader;
	};
}