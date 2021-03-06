#include "TextureBase.h"

#include <stb/stb_image.h>
#include <string.h>
#include <stdlib.h>
#include <gli/gli.hpp>

#include <bento.h>

namespace bento
{

	TextureBase::TextureBase
	( 
		GLenum _target /* = -1 */,
		GLenum _internalFormat /* = GL_RGBA8 */, 
		GLenum _magFilter /* = GL_LINEAR */, 
		GLenum _minFilter /* = GL_LINEAR */, 
		GLenum _wrapModeS /* = GL_REPEAT */, 
		GLenum _wrapModeT /* = GL_REPEAT */ 
	)
		: m_target(_target)
		, m_internalFormat(_internalFormat)
		, m_texture(-1)
		, m_magFilter(_magFilter)
		, m_minFilter(_minFilter)
		, m_wrapModeS(_wrapModeS)
		, m_wrapModeT(_wrapModeT)
	{

	}

	TextureBase::~TextureBase()
	{
		Invalidate();
	}

	void TextureBase::OnInvalidate()
	{
		if (glIsTexture(m_texture))
		{
			glDeleteTextures(1, &m_texture);
			m_texture = -1;
		}
	}

	void TextureBase::Validate()
	{
		assert(glIsTexture(m_texture) == false);

		glGenTextures(1, &m_texture);

		GL_CHECK(glBindTexture(m_target, m_texture));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_S, m_wrapModeS));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_T, m_wrapModeT));
		GL_CHECK(glBindTexture(m_target, GL_NONE));
	}

	GLenum TextureBase::MagFilter()
	{
		return m_magFilter;
	}

	void TextureBase::MagFilter(GLenum _value)
	{
		if (m_magFilter == _value) return;
		m_magFilter = _value;
		Invalidate();
	}

	GLuint TextureBase::MinFilter()
	{
		return m_minFilter;
	}

	void TextureBase::MinFilter(GLenum _value)
	{
		if (m_minFilter == _value) return;
		m_minFilter = _value;
		Invalidate();
	}

	GLenum TextureBase::WrapModeS()
	{
		return m_wrapModeS;
	}

	void TextureBase::WrapModeS(GLenum _value)
	{
		if (m_wrapModeS == _value)
			return;
		m_wrapModeS = _value;
		Invalidate();
	}

	GLenum TextureBase::WrapModeT()
	{
		return m_wrapModeT;
	}

	void TextureBase::WrapModeT(GLenum _value)
	{
		if (m_wrapModeT == _value)
			return;
		m_wrapModeT = _value;
		Invalidate();
	}

	GLuint TextureBase::TextureName()
	{
		ValidateNow();
		return m_texture;
	}

	GLenum TextureBase::Target()
	{
		return m_target;
	}

	GLenum TextureBase::InternalFormat()
	{
		return m_internalFormat;
	}
}