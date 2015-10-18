#include "TextureSquare.h"

#include <assert.h>
#include <Kaiga\Util\TextureUtil.h>

Kaiga::TextureSquare::TextureSquare()
{
}

Kaiga::TextureSquare::~TextureSquare()
{
}

void Kaiga::TextureSquare::SetSize(int _size)
{
	if (m_size == _size)
		return;
	m_size = _size;
	Invalidate();
}

int Kaiga::TextureSquare::GetSize()
{
	return 0;
}

int Kaiga::TextureSquare::GetNumMipMaps()
{
	if (m_minFilter == GL_NEAREST_MIPMAP_LINEAR || 
		m_minFilter == GL_NEAREST_MIPMAP_NEAREST ||
		m_minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		m_minFilter == GL_LINEAR_MIPMAP_LINEAR)
	{
		return TextureUtil::GetNumMipMaps(m_size);
	}

	return 1;
}

void Kaiga::TextureSquare::OnInvalidate()
{
	if (glIsTexture(m_name))
	{
		GL_CHECK(glDeleteTextures(1, &m_name));
		m_name = -1;
	}
}

void Kaiga::TextureSquare::Validate()
{
	assert(glIsTexture(m_name) == false);

	glGenTextures(1, &m_name);

	int numMipMaps = GetNumMipMaps();
	int d = m_size;
	for (int i = 0; i < numMipMaps; i++)
	{
		GL_CHECK(glTexImage2D
		(
			GL_TEXTURE_2D, i, m_pixelInternalFormat,
			d, d, 0, GL_RGBA, GL_FLOAT, NULL
		));
		d >>= 1;
	}

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_name));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter));
	GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter));
	GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, m_wrapModeR));
	GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapModeS));
	GL_CHECK( glBindTexture(GL_TEXTURE_2D, GL_NONE));
}
