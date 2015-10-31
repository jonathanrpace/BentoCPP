#include "TextureSquare.h"

#include <assert.h>

#include <bento\Util\GLErrorUtil.h>
#include <bento\Util\TextureUtil.h>

bento::TextureSquare::TextureSquare()
	: TextureBase(GL_TEXTURE_2D)
{
}

bento::TextureSquare::~TextureSquare()
{
}

int bento::TextureSquare::GetNumMipMaps()
{
	if (m_minFilter == GL_NEAREST_MIPMAP_LINEAR || 
		m_minFilter == GL_NEAREST_MIPMAP_NEAREST ||
		m_minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		m_minFilter == GL_LINEAR_MIPMAP_LINEAR)
	{
		return bento::TextureUtil::GetNumMipMaps(m_width);
	}

	return 1;
}

void bento::TextureSquare::Validate()
{
	assert(glIsTexture(m_texture) == false);

	glGenTextures(1, &m_texture);

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_texture));

	int numMipMaps = GetNumMipMaps();
	int d = m_width;
	for (int i = 0; i < numMipMaps; i++)
	{
		GL_CHECK(glTexImage2D
		(
			GL_TEXTURE_2D, i, m_format,
			d, d, 0, GL_RGBA, GL_FLOAT, NULL
		));
		d >>= 1;
	}

	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, m_wrapModeR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapModeS));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, GL_NONE));
}
