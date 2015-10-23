#pragma once

#include <vector>
#include <assert.h>

#include <glew.h>
#include <ramen.h>
#include <kaiga.h>
#include <kaiga/Textures/RectangleTexture.h>

namespace Kaiga
{
	class RenderTargetBase 
		: public Ramen::AbstractValidatable
		, public IRenderTarget
	{
	public:
		RenderTargetBase
		(
			int _width,
			int _height,
			bool _isRectangular,
			bool _hasDepthStencil = true,
			GLenum _internalFormat = GL_RGBA16F,
			GLenum _depthStencilFormat = GL_DEPTH24_STENCIL8
		);
		~RenderTargetBase();

		void SetSize(int _width, int _height);
		virtual void Bind() override;

	protected:
		const int MAX_ATTACHMENTS = 8;

		bool m_isRectangular;
		bool m_hasDepthStencil;
		GLenum m_internalFormat;
		GLenum m_depthStencilFormat;
		GLuint m_frameBuffer;
		int m_width;
		int m_height;

		std::vector<ITexture2D*> m_texturesByAttachment;
		std::vector<int> m_levelsByAttachment;

		virtual void Validate() override;
		virtual void OnInvalidate() override;
		void AttachTexture(int _index, RectangleTexture* _texture, int _level = 0);
	};
}