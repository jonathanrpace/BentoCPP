#pragma once

#include <Kaiga/Core/ShaderBase.h>
#include <Kaiga/ShaderStages/ScreenQuadVert.h>
#include <Kaiga/ShaderStages/RectTextureFrag.h>
#include <Kaiga/Geometry/ScreenQuadGeometry.h>
#include <Kaiga/Textures/RectangleTexture.h>

namespace Kaiga
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