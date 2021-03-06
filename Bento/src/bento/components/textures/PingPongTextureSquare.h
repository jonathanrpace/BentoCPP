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
			, GLenum _format = GL_RGBA
			, GLenum _type = GL_FLOAT
			, GLenum _magFilter = GL_LINEAR
			, GLenum _minFilter = GL_LINEAR
			, GLenum _wrapModeS = GL_REPEAT
			, GLenum _wrapModeT = GL_REPEAT
		);

		void Swap();
		TextureSquare& GetRead() { return *m_readTexture; }
		TextureSquare& GetWrite() { return *m_writeTexture; }
		void Size(int _size);
		int Size() { return m_textureA.Size(); }

	private:
		TextureSquare m_textureA;
		TextureSquare m_textureB;
		TextureSquare* m_readTexture;
		TextureSquare* m_writeTexture;
	};
}