#pragma once

#include <bento/core/RenderTargetBase.h>
#include <bento/components/textures/RectangleTexture.h>

namespace godBox
{
	class RenderTarget : public bento::RenderTargetBase
	{
	public:
		static const GLuint FRAME_BUFFER_ATTACHMENT_POSITION;
		static const GLuint FRAME_BUFFER_ATTACHMENT_COLOR;
		static const GLuint FRAME_BUFFER_ATTACHMENT_NORMAL;
		static const GLuint FRAME_BUFFER_ATTACHMENT_COLOR_POST_TRANSPARENCY;
		static const GLuint FRAME_BUFFER_ATTACHMENT_FILTERED_COLOR;
		
		RenderTarget(int _width = 800, int _height = 600);
		~RenderTarget();

		void Clear();
		void BindForForwardPhase();
		void BindForTransparentPhase();
		
		bento::RectangleTexture& PositionTexture();
		bento::RectangleTexture& ColorTexture();
		bento::RectangleTexture& BlurredColorTextureA();
		bento::RectangleTexture& BlurredColorTextureB();
		bento::RectangleTexture& NormalTexture();
		bento::RectangleTexture& ColorPostTransparencyTexture();
		bento::TextureSquare& FilteredColorTexture();
		
	private:
		bento::RectangleTexture m_positionTexture;
		bento::RectangleTexture m_colorTexture;
		bento::RectangleTexture m_blurredColorTextureA;
		bento::RectangleTexture m_blurredColorTextureB;
		bento::RectangleTexture m_normalTexture;
		bento::RectangleTexture m_colorPostTransparencyTexture;
		bento::TextureSquare m_filteredColorTexture;
	};
}