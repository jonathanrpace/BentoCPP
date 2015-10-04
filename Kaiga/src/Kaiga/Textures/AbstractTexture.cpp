#include "AbstractTexture.h"

Kaiga::AbstractTexture::AbstractTexture() :
	m_minFilter(GL_LINEAR),
	m_magFilter(GL_LINEAR),
	m_wrapModeR(GL_REPEAT),
	m_wrapModeS(GL_REPEAT),
	m_pixelInternalFormat(GL_RGBA)
{
}

Kaiga::AbstractTexture::~AbstractTexture()
{
}

GLuint Kaiga::AbstractTexture::GetName()
{
	return m_name;
}

GLenum Kaiga::AbstractTexture::GetMinFilter()
{
	return m_minFilter;
}

void Kaiga::AbstractTexture::SetMinFilter(GLenum _value)
{
	if (m_minFilter == _value)
		return;
	m_minFilter = _value;
	Invalidate();
}

GLenum Kaiga::AbstractTexture::GetMagFilter()
{
	return m_magFilter;
}

void Kaiga::AbstractTexture::SetMagFilter(GLenum _value)
{
	if (m_magFilter == _value)
		return;
	m_magFilter = _value;
	Invalidate();
}

GLenum Kaiga::AbstractTexture::GetWrapModeS()
{
	return m_wrapModeS;
}

void Kaiga::AbstractTexture::SetWrapModeS(GLenum _value)
{
	if (m_wrapModeS == _value)
		return;
	m_wrapModeS = _value;
	Invalidate();
}

GLenum Kaiga::AbstractTexture::GetWrapModeR()
{
	return m_wrapModeR;
}

void Kaiga::AbstractTexture::SetWrapModeR(GLenum _value)
{
	if (m_wrapModeR == _value)
		return;
	m_wrapModeR = _value;
	Invalidate();
}

GLenum Kaiga::AbstractTexture::GetPixelInternalFormat()
{
	return m_pixelInternalFormat;
}

void Kaiga::AbstractTexture::SetPixelInternalFormat(GLenum _value)
{
	if (m_pixelInternalFormat == _value)
		return;
	m_pixelInternalFormat = _value;
	Invalidate();
}
