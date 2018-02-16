#pragma once

#include <map>

#include <bento.h>
#include <bento/core/AbstractValidatable.h>
#include <bento/core/IRenderTarget.h>
#include <bento/components/textures/RectangleTexture.h>
#include <bento/components/textures/TextureSquare.h>

namespace bento
{
	class RenderTargetBase 
		: public bento::AbstractValidatable
		, public IRenderTarget
	{
	public:
		RenderTargetBase
		(
			int _width,
			int _height,
			bool _isRectangular,
			bool _hasDepthStencil = true,
			GLenum _depthStencilFormat = GL_DEPTH24_STENCIL8
		);
		~RenderTargetBase();

		void SetSize(int _width, int _height);
		virtual void Bind() override;

		void AttachTexture(GLenum _attachment, RectangleTexture& _texture, int _level = 0);
		void AttachTexture(GLenum _attachment, TextureSquare& _texture, int _level = 0);

		void SetDrawTextures( TextureSquare& _texture0 );
		void SetDrawTextures( TextureSquare& _texture0, TextureSquare& _texture1 );
		void SetDrawTextures( TextureSquare& _texture0, TextureSquare& _texture1, TextureSquare& _texture2 );
		void SetDrawTextures( TextureSquare& _texture0, TextureSquare& _texture1, TextureSquare& _texture2, TextureSquare& _texture3 );

		void SetDrawTextures( RectangleTexture& _texture0 );
		void SetDrawTextures( RectangleTexture& _texture0, RectangleTexture& _texture1 );
		void SetDrawTextures( RectangleTexture& _texture0, RectangleTexture& _texture1, RectangleTexture& _texture2 );
		void SetDrawTextures( RectangleTexture& _texture0, RectangleTexture& _texture1, RectangleTexture& _texture2, RectangleTexture& _texture3 );

		void SetDrawBuffers(const GLenum* const _drawBuffers, GLsizei _size);

		int Width() { return m_width; }
		int Height() { return m_height; }

	protected:
		const int MAX_ATTACHMENTS = 8;

		bool m_isRectangular;
		bool m_hasDepthStencil;
		GLenum m_depthStencilFormat;
		GLuint m_frameBuffer;
		GLuint m_depthBuffer;
		int m_width;
		int m_height;

		std::map<GLenum, ITexture*> m_texturesByAttachment;
		std::map<GLenum, int> m_levelsByAttachment;

		virtual void Validate() override;
		virtual void OnInvalidate() override;
	};
}