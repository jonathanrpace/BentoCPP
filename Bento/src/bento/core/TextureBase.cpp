#include "TextureBase.h"

#include <bento\Util\GLErrorUtil.h>

bento::TextureBase::TextureBase(
	GLenum _textureTarget,
	int _width,
	int _height,
	GLenum _format,
	GLenum _magFilter,
	GLenum _minFilter,
	GLenum _wrapModeR,
	GLenum _wrapModeS
	)
	: m_textureTarget(_textureTarget)
	, m_width(_width)
	, m_height(_height)
	, m_format(_format)
	, m_magFilter(_magFilter)
	, m_minFilter(_minFilter)
	, m_wrapModeR(_wrapModeR)
	, m_wrapModeS(_wrapModeS)
	, m_texture(-1)
{

}

bento::TextureBase::~TextureBase()
{
	Invalidate();
}

void bento::TextureBase::TexImage2D(int _level, GLenum _format, GLenum _type, const GLvoid * _data)
{
	ValidateNow();
	GL_CHECK(glBindTexture(m_textureTarget, m_texture));
	GL_CHECK(glTexImage2D(m_textureTarget, _level, m_format, m_width, m_height, 0, _format, _type, _data));
}

void bento::TextureBase::OnInvalidate()
{
	if (glIsTexture(m_texture))
	{
		glDeleteTextures(1, &m_texture);
		m_texture = -1;
	}
}

int bento::TextureBase::Width()
{
	return m_width;
}

void bento::TextureBase::Width(int _value)
{
	if (m_width == _value) return;
	m_width = _value;
	Invalidate();
}

int bento::TextureBase::Height()
{
	return m_height;
}

void bento::TextureBase::Height(int _value)
{
	if (m_height == _value) return;
	m_height = _value;
	Invalidate();
}

GLenum bento::TextureBase::Format()
{
	return m_format;
}

void bento::TextureBase::Format(GLenum _value)
{
	if (m_format == _value) return;
	m_format = _value;
	Invalidate();
}

GLenum bento::TextureBase::MagFilter()
{
	return m_magFilter;
}

void bento::TextureBase::MagFilter(GLenum _value)
{
	if (m_magFilter == _value) return;
	m_magFilter = _value;
	Invalidate();
}

GLuint bento::TextureBase::MinFilter()
{
	return m_minFilter;
}

void bento::TextureBase::MinFilter(GLenum _value)
{
	if (m_minFilter == _value) return;
	m_minFilter = _value;
	Invalidate();
}

GLenum bento::TextureBase::WrapModeS()
{
	return m_wrapModeS;
}

void bento::TextureBase::WrapModeS(GLenum _value)
{
	if (m_wrapModeS == _value)
		return;
	m_wrapModeS = _value;
	Invalidate();
}

GLenum bento::TextureBase::WrapModeR()
{
	return m_wrapModeR;
}

void bento::TextureBase::WrapModeR(GLenum _value)
{
	if (m_wrapModeR == _value)
		return;
	m_wrapModeR = _value;
	Invalidate();
}

void bento::TextureBase::SetSize(int _width, int _height)
{
	Width(_width);
	Height(_height);
}

GLuint bento::TextureBase::TextureName()
{
	ValidateNow();
	return m_texture;
}
