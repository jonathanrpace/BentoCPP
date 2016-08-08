#include "PingPongTextureSquare.h"

namespace bento
{

	PingPongTextureSquare::PingPongTextureSquare
	(
		  int _size /*= 256 */
		, GLenum _internalFormat /*= GL_RGBA8 */
		, GLenum _magFilter /*= GL_LINEAR */
		, GLenum _minFilter /*= GL_LINEAR */
		, GLenum _wrapModeR /*= GL_REPEAT */
		, GLenum _wrapModeS /*= GL_REPEAT */
	) 
		: m_textureA(_size, _internalFormat, _magFilter, _minFilter, _wrapModeR, _wrapModeS )
		, m_textureB(_size, _internalFormat, _magFilter, _minFilter, _wrapModeR, _wrapModeS )
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
}
