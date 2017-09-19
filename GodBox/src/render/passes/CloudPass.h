#pragma once

// bento
#include <bento/core/NodeGroupProcess.h>
#include <bento/core/RenderPass.h>
#include <bento/core/SharedObject.h>
#include <bento/core/ShaderBase.h>
#include <bento/core/ShaderStageBase.h>
#include <bento/components/Transform.h>
#include <bento/components/geom/Geometry.h>

// app
#include <components/materials/CloudMaterial.h>

using namespace bento;

namespace godBox
{
	DEFINE_NODE_3
	(
		CloudPassNode,
		Geometry, geom,
		Transform, transform,
		CloudMaterial, material
	)

	class CloudPass
		: public NodeGroupProcess<CloudPassNode>
		, public RenderPass
		, public SharedObject<CloudPass>
	{
	private:
		struct VertShader : ShaderStageBase
		{
			VertShader();
			void BindPerModel(CloudMaterial&);
		};

		struct FragShader : ShaderStageBase
		{
			FragShader();
			void BindPerModel(CloudMaterial&);
		};

		struct Shader : ShaderBase<VertShader, FragShader> {};

		

	public:
		CloudPass(std::string _name = "CloudPass");

		// From Process
		virtual void Advance(double _dt) override;

	private:
		Shader m_shader;
	};
}