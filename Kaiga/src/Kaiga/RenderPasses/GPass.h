#pragma once

#include <Ramen/Core/SharedObject.h>

#include <Kaiga/Core/NodeGroupRenderPassBase.h>
#include <Kaiga/Core/ShaderBase.h>
#include <Kaiga/Core/ShaderStageBase.h>
#include <Kaiga/Core/GeometryBase.h>
#include <Kaiga/Materials/StandardMaterial.h>
#include <Kaiga/Components/Transform.h>

namespace Kaiga
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
		: public Ramen::SharedObject<GPass>
		, public NodeGroupRenderPassBase<GPassNode>
	{
	public:
		virtual void Render();
		virtual RenderPhase GetRenderPhase() override;

	private:
		GShader m_shader;
	};
}