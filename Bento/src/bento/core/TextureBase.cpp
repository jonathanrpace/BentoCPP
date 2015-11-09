#include "TextureBase.h"

#include <stb/stb_image.h>
#include <string.h>
#include <stdlib.h>

#include <bento.h>

namespace bento
{

	TextureBase::TextureBase
	(
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

	TextureBase::~TextureBase()
	{
		Invalidate();
	}

	void TextureBase::TexImage2D(GLenum _format, GLenum _type, const GLvoid * _data, int _level) /* _level = 0 */
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_textureTarget, m_texture));
		GL_CHECK(glTexImage2D(m_textureTarget, _level, m_format, m_width, m_height, 0, _format, _type, _data));
	}

	void TextureBase::TexImage2D(const char* _filename) /* _level = 0 */
	{
		size_t resolvedFilenameSize = strlen(_filename) + strlen(bento::Config::ResourcePath()) + 1;
		char* resolvedFilename = new char[resolvedFilenameSize];
		strcpy_s(resolvedFilename, resolvedFilenameSize, bento::Config::ResourcePath())  ;
		strcat_s(resolvedFilename, resolvedFilenameSize, _filename);
		
		int x, y, n;
		unsigned char *data = stbi_load(resolvedFilename, &x, &y, &n, 4);
		m_width = x;
		m_height = y;
		m_format = GL_RGBA8;

		TexImage2D(GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
		delete resolvedFilename;
	}

	void TextureBase::GetTexImage(GLint _level, GLenum _format, GLenum _type, GLvoid* _pixels)
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_textureTarget, m_texture));
		GL_CHECK(glGetTexImage(m_textureTarget, _level, _format, _type, _pixels));
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

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_texture));
		GL_CHECK(glBindTexture(m_textureTarget, m_texture));
		GL_CHECK(glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, m_magFilter));
		GL_CHECK(glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, m_minFilter));
		GL_CHECK(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_R, m_wrapModeR));
		GL_CHECK(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, m_wrapModeS));
		GL_CHECK(glBindTexture(m_textureTarget, GL_NONE));
	}

	int TextureBase::Width()
	{
		return m_width;
	}

	void TextureBase::Width(int _value)
	{
		if (m_width == _value) return;
		m_width = _value;
		Invalidate();
	}

	int TextureBase::Height()
	{
		return m_height;
	}

	void TextureBase::Height(int _value)
	{
		if (m_height == _value) return;
		m_height = _value;
		Invalidate();
	}

	GLenum TextureBase::Format()
	{
		return m_format;
	}

	void TextureBase::Format(GLenum _value)
	{
		if (m_format == _value) return;
		m_format = _value;
		Invalidate();
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

	GLenum TextureBase::WrapModeR()
	{
		return m_wrapModeR;
	}

	void TextureBase::WrapModeR(GLenum _value)
	{
		if (m_wrapModeR == _value)
			return;
		m_wrapModeR = _value;
		Invalidate();
	}

	void TextureBase::SetSize(int _width, int _height)
	{
		Width(_width);
		Height(_height);
	}

	GLuint TextureBase::TextureName()
	{
		ValidateNow();
		return m_texture;
	}
}