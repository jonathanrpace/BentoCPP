#pragma once

#include <gl/glew.h>

#include <bento/components/textures/TextureSquare.h>

namespace bento
{
	class PingPongTextureSquare
	{
	public:
		PingPongTextureSquare( 
			int _size = 256
			, GLenum _internalFormat = GL_RGBA8
			, GLenum _magFilter = GL_LINEAR
			, GLenum _minFilter = GL_LINEAR
			, GLenum _wrapModeR = GL_REPEAT
			, GLenum _wrapModeS = GL_REPEAT
		);

		void Swap();
		TextureSquare& GetRead() { return *m_readTexture; }
		TextureSquare& GetWrite() { return *m_writeTexture; }

	private:
		TextureSquare m_textureA;
		TextureSquare m_textureB;
		TextureSquare* m_readTexture;
		TextureSquare* m_writeTexture;
	};
}