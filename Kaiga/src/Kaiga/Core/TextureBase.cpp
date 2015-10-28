#include "TextureBase.h"

#include <Kaiga\Util\GLErrorUtil.h>

Kaiga::TextureBase::TextureBase(
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

Kaiga::TextureBase::~TextureBase()
{
	Invalidate();
}

void Kaiga::TextureBase::TexImage2D(int _level, GLenum _format, GLenum _type, const GLvoid * _data)
{
	ValidateNow();
	GL_CHECK(glBindTexture(m_textureTarget, m_texture));
	GL_CHECK(glTexImage2D(m_textureTarget, _level, m_format, m_width, m_height, 0, _format, _type, _data));
}

void Kaiga::TextureBase::OnInvalidate()
{
	if (glIsTexture(m_texture))
	{
		glDeleteTextures(1, &m_texture);
		m_texture = -1;
	}
}

int Kaiga::TextureBase::Width()
{
	return m_width;
}

void Kaiga::TextureBase::Width(int _value)
{
	if (m_width == _value) return;
	m_width = _value;
	Invalidate();
}

int Kaiga::TextureBase::Height()
{
	return m_height;
}

void Kaiga::TextureBase::Height(int _value)
{
	if (m_height == _value) return;
	m_height = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::Format()
{
	return m_format;
}

void Kaiga::TextureBase::Format(GLenum _value)
{
	if (m_format == _value) return;
	m_format = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::MagFilter()
{
	return m_magFilter;
}

void Kaiga::TextureBase::MagFilter(GLenum _value)
{
	if (m_magFilter == _value) return;
	m_magFilter = _value;
	Invalidate();
}

GLuint Kaiga::TextureBase::MinFilter()
{
	return m_minFilter;
}

void Kaiga::TextureBase::MinFilter(GLenum _value)
{
	if (m_minFilter == _value) return;
	m_minFilter = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::WrapModeS()
{
	return m_wrapModeS;
}

void Kaiga::TextureBase::WrapModeS(GLenum _value)
{
	if (m_wrapModeS == _value)
		return;
	m_wrapModeS = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::WrapModeR()
{
	return m_wrapModeR;
}

void Kaiga::TextureBase::WrapModeR(GLenum _value)
{
	if (m_wrapModeR == _value)
		return;
	m_wrapModeR = _value;
	Invalidate();
}

void Kaiga::TextureBase::SetSize(int _width, int _height)
{
	Width(_width);
	Height(_height);
}

GLuint Kaiga::TextureBase::TextureName()
{
	ValidateNow();
	return m_texture;
}
