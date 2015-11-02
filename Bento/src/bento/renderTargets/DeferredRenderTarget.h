#pragma once

#include <bento/core/RenderTargetBase.h>
#include <bento/textures/RectangleTexture.h>

namespace bento
{
	class DeferredRenderTarget : public RenderTargetBase
	{
	public:
		static const GLuint FRAME_BUFFER_ATTACHMENT_POSITION;
		static const GLuint FRAME_BUFFER_ATTACHMENT_NORMAL;
		static const GLuint FRAME_BUFFER_ATTACHMENT_ALBEDO;
		static const GLuint FRAME_BUFFER_ATTACHMENT_MATERIAL;
		static const GLuint FRAME_BUFFER_ATTACHMENT_DIRECT_LIGHT;
		static const GLuint FRAME_BUFFER_ATTACHMENT_INDIRECT_LIGHT;
		static const GLuint FRAME_BUFFER_ATTACHMENT_OUTPUT;
		
		DeferredRenderTarget(int _width = 800, int _height = 600);
		~DeferredRenderTarget();

		void Clear();
		void BindForGPhase();
		void BindForDirectLightPhase();
		void BindForIndirectLightPhase();
		void BindForResolvePhase();
		void BindForNoDraw();
		
		RectangleTexture* NormalTexture();
		RectangleTexture* PositionTexture();
		RectangleTexture* AlbedoTexture();
		RectangleTexture* MaterialTexture();
		RectangleTexture* DirectLightTexture();
		RectangleTexture* IndirectLightTexture();
		RectangleTexture* OutputTexture();
		
	private:
		RectangleTexture m_normalTexture;
		RectangleTexture m_positionTexture;
		RectangleTexture m_albedoTexture;
		RectangleTexture m_materialTexture;
		RectangleTexture m_directLightTexture;
		RectangleTexture m_indirectLightTexture;
		RectangleTexture m_outputTexture;
	};
}