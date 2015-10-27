#pragma once

#include <map>
#include <assert.h>

#include <glew.h>

#include <Ramen/Core/AbstractValidatable.h>

#include <Kaiga/Core/IRenderTarget.h>
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
		GLuint m_depthBuffer;
		int m_width;
		int m_height;

		std::map<GLenum, ITexture2D*> m_texturesByAttachment;
		std::map<GLenum, int> m_levelsByAttachment;

		virtual void Validate() override;
		virtual void OnInvalidate() override;
		void AttachTexture(GLenum _attachment, RectangleTexture* _texture, int _level = 0);
	};
}