#pragma once

#include <bento/core/ITextureSquare.h>
#include <bento/core/TextureBase.h>

namespace bento
{
	class TextureSquare : public ITextureSquare, public TextureBase
	{
	public:
		TextureSquare
		(
			int _size = 256
			, GLenum _internalFormat = GL_RGBA8
			, GLenum _magFilter = GL_NEAREST
			, GLenum _minFilter = GL_NEAREST
			, GLenum _wrapModeR = GL_REPEAT
			, GLenum _wrapModeS = GL_REPEAT
		);
		~TextureSquare();

		virtual void GenerateMipMaps();
		int GetNumMipMaps();
	protected:
		// Inherited via AbstractValidatable
		virtual void Validate() override;
	};
}