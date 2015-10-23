#include <kaiga.h>

#include <Kaiga/Textures/RectangleTexture.h>

#include <vector>

namespace Kaiga
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
		
		static const GLuint DRAW_BUFFER_POSITION;
		static const GLuint DRAW_BUFFER_NORMAL;
		static const GLuint DRAW_BUFFER_ALBEDO;
		static const GLuint DRAW_BUFFER_MATERIAL;
		static const GLuint DRAW_BUFFER_DIRECT_LIGHT;
		static const GLuint DRAW_BUFFER_INDIRECT_LIGHT;
		static const GLuint DRAW_BUFFER_OUTPUT;
		
		DeferredRenderTarget(int _width = 800, int _height = 600);

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

		static const std::vector<GLenum> ALL_DRAW_BUFFERS;
		static const std::vector<GLenum> G_PHASE_DRAW_BUFFERS;
		static const std::vector<GLenum> DIRECT_LIGHT_PHASE_DRAW_BUFFERS;
		static const std::vector<GLenum> INDIRECT_LIGHT_PHASE_DRAW_BUFFERS;
		static const std::vector<GLenum> RESOLVE_PHASE_DRAW_BUFFERS;

		RectangleTexture m_normalTexture;
		RectangleTexture m_positionTexture;
		RectangleTexture m_albedoTexture;
		RectangleTexture m_materialTexture;
		RectangleTexture m_directLightTexture;
		RectangleTexture m_indirectLightTexture;
		RectangleTexture m_outputTexture;
	};
}