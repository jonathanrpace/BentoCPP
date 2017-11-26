#include "TextureSquare.h"

#include <assert.h>

#include <bento\Util\GLErrorUtil.h>
#include <bento\Util\TextureUtil.h>

namespace bento
{
	TextureSquare::TextureSquare
	( 
		 int _size				/* = 256 */
		,GLenum _internalFormat	/* = GL_RGBA8 */
		,GLenum _format			/* = GL_RGBA */
		,GLenum _type			/* = GL_FLOAT */
		,GLenum _magFilter		/* = GL_LINEAR */
		,GLenum _minFilter		/* = GL_LINEAR */
		,GLenum _wrapModeS		/* = GL_REPEAT */
		,GLenum _wrapModeT		/* = GL_REPEAT */ 
	)
		: TextureBase(GL_TEXTURE_2D, _internalFormat, _magFilter, _minFilter, _wrapModeS, _wrapModeT)
		, m_size(_size)
		, m_format(_format)
		, m_type(_type)
	{
	}

	TextureSquare::~TextureSquare()
	{
	}

	void TextureSquare::TexImage2D(const GLvoid * _data, int _level)
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_target, m_texture));
		GL_CHECK(glTexImage2D(m_target, _level, m_internalFormat, m_size, m_size, 0, m_format, m_type, _data));
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
				glTexImage2D(m_target, i, m_internalFormat, d, d, 0, m_format, m_type, NULL)
			);
			d >>= 1;
		}

		GL_CHECK(glBindTexture(m_target, GL_NONE));
	}
}