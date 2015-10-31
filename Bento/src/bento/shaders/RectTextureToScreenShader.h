#pragma once

#include <bento/core/ShaderBase.h>
#include <bento/shaderStages/ScreenQuadVert.h>
#include <bento/shaderStages/RectTextureFrag.h>
#include <bento/geom/ScreenQuadGeometry.h>
#include <bento/textures/RectangleTexture.h>

namespace bento
{
	class RectTextureToScreenShader 
		: public ShaderBase<ScreenQuadVert, RectTextureFrag>
	{
		public:
			void Render(RectangleTexture* _texture);

		private:
			ScreenQuadGeometry m_screenQuadGeom;
	};
}