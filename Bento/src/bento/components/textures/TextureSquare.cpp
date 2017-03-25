#include "TextureSquare.h"

#include <assert.h>

#include <bento\Util\GLErrorUtil.h>
#include <bento\Util\TextureUtil.h>

namespace bento
{
	TextureSquare::TextureSquare
	( 
		int _size /* = 256  */, 
		GLenum _format /* = GL_RGBA8  */, 
		GLenum _magFilter /* = GL_LINEAR  */, 
		GLenum _minFilter /* = GL_LINEAR  */, 
		GLenum _wrapModeR /* = GL_REPEAT  */, 
		GLenum _wrapModeS /* = GL_REPEAT */ 
	)
		: TextureBase(GL_TEXTURE_2D, _format, _magFilter, _minFilter, _wrapModeS, _wrapModeR)
		, m_size(_size)
	{
	}

	TextureSquare::~TextureSquare()
	{
	}

	void TextureSquare::TexImage2D(GLenum _format, GLenum _type, const GLvoid * _data, int _level)
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_target, m_texture));
		GL_CHECK(glTexImage2D(m_target, _level, m_format, m_size, m_size, 0, _format, _type, _data));
	}

	void TextureSquare::Size(int _size)
	{
		if ( _size == m_size )
			return;
		m_size = _size;
		Invalidate();
	}

	void TextureSquare::GenerateMipMaps()
	{
		GL_CHECK(glGenerateTextureMipmap(TextureName()));
	}

	int TextureSquare::GetNumMipMaps()
	{
		if (m_minFilter == GL_NEAREST_MIPMAP_LINEAR ||
			m_minFilter == GL_NEAREST_MIPMAP_NEAREST ||
			m_minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			m_minFilter == GL_LINEAR_MIPMAP_LINEAR)
		{
			return bento::textureUtil::GetNumMipMaps(m_size);
		}

		return 1;
	}

	void TextureSquare::Validate()
	{
		TextureBase::Validate();

		GL_CHECK(glBindTexture(m_target, m_texture));

		int numMipMaps = GetNumMipMaps();
		int d = m_size;
		for (int i = 0; i < numMipMaps; i++)
		{
			GL_CHECK(
				glTexImage2D(m_target, i, m_format, d, d, 0, GL_RGBA, GL_FLOAT, NULL)
			);
			d >>= 1;
		}

		GL_CHECK(glBindTexture(m_target, GL_NONE));
	}
}