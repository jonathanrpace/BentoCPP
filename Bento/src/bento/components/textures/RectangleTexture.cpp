#include "RectangleTexture.h"

#include <bento.h>

namespace bento
{
	RectangleTexture::RectangleTexture(
		int _width, 
		int _height, 
		GLuint _format, 
		GLuint _magFilter, 
		GLuint _minFilter
	) 
		: TextureBase(GL_TEXTURE_RECTANGLE, _format, _magFilter, _minFilter)
		, m_width(_width)
		, m_height(_height)
	{

	}

	void RectangleTexture::TexImage2D(GLenum _format, GLenum _type, const GLvoid * _data)
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_target, m_texture));
		GL_CHECK(glTexImage2D(m_target, 0, m_internalFormat, m_width, m_height, 0, _format, _type, _data));
	}

	void RectangleTexture::Width(int _width)
	{
		if ( m_width == _width )
			return;
		m_width = _width;
		Invalidate();
	}

	void RectangleTexture::Height(int _height)
	{
		if ( m_height == _height )
			return;
		m_height = _height;
		Invalidate();
	}

	void RectangleTexture::Validate()
	{
		GL_CHECK(glGenTextures(1, &m_texture));
		GL_CHECK(glBindTexture(m_target, m_texture));
		GL_CHECK(glTexImage2D(m_target, 0, m_internalFormat, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CHECK(glBindTexture(m_target, 0));
	}
}