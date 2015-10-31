#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/NodeGroupRenderPassBase.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/core/GeometryBase.h>
#include <bento/components/Transform.h>
#include <bento/materials/StandardMaterial.h>

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
		GeometryBase, geom,
		Transform, transform,
		StandardMaterial, material
	)

	class GPass
		: public bento::SharedObject<GPass>
		, public NodeGroupRenderPassBase<GPassNode>
	{
	public:
		virtual void Render();
		virtual RenderPhase GetRenderPhase() override;

	private:
		GShader m_shader;
	};
}