#pragma once

#include <bento/core/ShaderBase.h>
#include <bento/render/shaders/ScreenQuadVert.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/textures/RectangleTexture.h>

namespace bento
{
	struct ResolveShaderFrag : ShaderStageBase
	{
		ResolveShaderFrag() 
			: ShaderStageBase("shaders/Resolve.frag") 
		{}
	};

	class ResolveShader 
		: public ShaderBase<ScreenQuadVert, ResolveShaderFrag>
	{
		public:
			void Render(RectangleTexture&, RectangleTexture&, ITexture&, float, float);

		private:
			ScreenQuadGeometry m_screenQuadGeom;
	};
}