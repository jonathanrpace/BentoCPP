#include "PingPongTextureSquare.h"

namespace bento
{

	PingPongTextureSquare::PingPongTextureSquare
	(
		  int _size /*= 256 */
		, GLenum _internalFormat /*= GL_RGBA8 */
		, GLenum _format /*= GL_RGBA */
		, GLenum _type /*= GL_FLOAT */
		, GLenum _magFilter /*= GL_LINEAR */
		, GLenum _minFilter /*= GL_LINEAR */
		, GLenum _wrapModeS /*= GL_REPEAT */
		, GLenum _wrapModeT /*= GL_REPEAT */
	) 
		: m_textureA(_size, _internalFormat, _format, _type, _magFilter, _minFilter, _wrapModeS, _wrapModeT )
		, m_textureB(_size, _internalFormat, _format, _type, _magFilter, _minFilter, _wrapModeS, _wrapModeT )
		, m_readTexture(&m_textureA)
		, m_writeTexture(&m_textureB)
	{

	}

	void PingPongTextureSquare::Swap()
	{
		TextureSquare* tmp = m_readTexture;
		m_readTexture = m_writeTexture;
		m_writeTexture = tmp;
	}

	void PingPongTextureSquare::Size(int _size)
	{
		m_textureA.Size(_size);
		m_textureB.Size(_size);
	}

}
