#include "TextureBase.h"

Kaiga::TextureBase::TextureBase() :
	m_minFilter(GL_LINEAR),
	m_magFilter(GL_LINEAR),
	m_wrapModeR(GL_REPEAT),
	m_wrapModeS(GL_REPEAT),
	m_pixelInternalFormat(GL_RGBA)
{
}

Kaiga::TextureBase::~TextureBase()
{
}

GLuint Kaiga::TextureBase::GetName()
{
	return m_name;
}

GLenum Kaiga::TextureBase::GetMinFilter()
{
	return m_minFilter;
}

void Kaiga::TextureBase::SetMinFilter(GLenum _value)
{
	if (m_minFilter == _value)
		return;
	m_minFilter = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::GetMagFilter()
{
	return m_magFilter;
}

void Kaiga::TextureBase::SetMagFilter(GLenum _value)
{
	if (m_magFilter == _value)
		return;
	m_magFilter = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::GetWrapModeS()
{
	return m_wrapModeS;
}

void Kaiga::TextureBase::SetWrapModeS(GLenum _value)
{
	if (m_wrapModeS == _value)
		return;
	m_wrapModeS = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::GetWrapModeR()
{
	return m_wrapModeR;
}

void Kaiga::TextureBase::SetWrapModeR(GLenum _value)
{
	if (m_wrapModeR == _value)
		return;
	m_wrapModeR = _value;
	Invalidate();
}

GLenum Kaiga::TextureBase::GetPixelInternalFormat()
{
	return m_pixelInternalFormat;
}

void Kaiga::TextureBase::SetPixelInternalFormat(GLenum _value)
{
	if (m_pixelInternalFormat == _value)
		return;
	m_pixelInternalFormat = _value;
	Invalidate();
}
