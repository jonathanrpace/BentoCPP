#include "TextureBase.h"

Kaiga::TextureBase::TextureBase(
	int _width,
	int _height,
	GLuint _format,
	GLuint _magFilter,
	GLuint _minFilter
	)
	: m_width(_width)
	, m_height(_height)
	, m_format(_format)
	, m_magFilter(_magFilter)
	, m_minFilter(_minFilter)
	, m_texture(-1)
{

}

Kaiga::TextureBase::~TextureBase()
{
	Invalidate();
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
