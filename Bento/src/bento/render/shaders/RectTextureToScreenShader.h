#pragma once

#include <bento/core/ShaderBase.h>
#include <bento/render/shaders/ScreenQuadVert.h>
#include <bento/render/shaders/RectTextureFrag.h>
#include <bento/components/geom/ScreenQuadGeometry.h>
#include <bento/components/textures/RectangleTexture.h>

namespace bento
{
	class RectTextureToScreenShader 
		: public ShaderBase<ScreenQuadVert, RectTextureFrag>
	{
		public:
			void Render(RectangleTexture& _texture, float _offset = 0.0f);

		private:
			ScreenQuadGeometry m_screenQuadGeom;
	};
}