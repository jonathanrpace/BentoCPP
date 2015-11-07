#include "TextureSquare.h"

#include <assert.h>

#include <bento\Util\GLErrorUtil.h>
#include <bento\Util\TextureUtil.h>

bento::TextureSquare::TextureSquare
(
	int _size,
	GLenum _format,
	GLenum _magFilter,
	GLenum _minFilter,
	GLenum _wrapModeR,
	GLenum _wrapModeS
)
	: TextureBase(GL_TEXTURE_2D, _size, _size, _format, _magFilter, _minFilter, _wrapModeS, _wrapModeR)
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
	TextureBase::Validate();

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

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, GL_NONE));
}
