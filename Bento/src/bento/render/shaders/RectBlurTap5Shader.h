#pragma once

#include <bento/core/ShaderBase.h>
#include <bento/render/shaders/ScreenQuadVert.h>
#include <bento/render/shaders/RectBlurTap5Frag.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/textures/RectangleTexture.h>

namespace bento
{
	class RectBlurTap5Shader 
		: public ShaderBase<ScreenQuadVert, RectBlurTap5Frag>
	{
		public:
			void Render(RectangleTexture& _texture);

		private:
			ScreenQuadGeometry m_screenQuadGeom;
	};
}