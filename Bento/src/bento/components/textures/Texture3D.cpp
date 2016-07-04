#include "Texture3D.h"

#include <stb/stb_image.h>
#include <string.h>
#include <stdlib.h>

#include <bento.h>

namespace bento
{
	Texture3D::Texture3D
	(
		int _width,
		int _height,
		int _depth,
		GLenum _format,
		GLenum _magFilter,
		GLenum _minFilter,
		GLenum _wrapModeR,
		GLenum _wrapModeS,
		GLenum _wrapModeT
	)
		: m_width(_width)
		, m_height(_height)
		, m_depth(_depth)
		, m_format(_format)
		, m_magFilter(_magFilter)
		, m_minFilter(_minFilter)
		, m_wrapModeR(_wrapModeR)
		, m_wrapModeS(_wrapModeS)
		, m_wrapModeT(_wrapModeT)
		, m_texture(-1)
	{

	}

	Texture3D::~Texture3D()
	{
		Invalidate();
	}

	void Texture3D::TexImage3D(GLenum _format, GLenum _type, const GLvoid * _data, int _level) /* _level = 0 */
	{
		ValidateNow();
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, m_texture));
		GL_CHECK(glTexImage3D(GL_TEXTURE_3D, _level, m_format, m_width, m_height, m_depth, 0, _format, _type, _data));
	}

	void Texture3D::TexImage3D(const char* _filename)
	{
		size_t resolvedFilenameSize = strlen(_filename) + strlen(bento::Config::ResourcePath().c_str()) + 1;
		char* resolvedFilename = new char[resolvedFilenameSize];
		strcpy_s(resolvedFilename, resolvedFilenameSize, bento::Config::ResourcePath().c_str());
		strcat_s(resolvedFilename, resolvedFilenameSize, _filename);
		
		int x, y, n;
		unsigned char *data = stbi_load(resolvedFilename, &x, &y, &n, 4);
		m_width = x;
		m_height = x;
		m_depth = y / m_width;
		
		m_format = GL_RGBA8;

		TexImage3D(GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
		delete[] resolvedFilename;
	}

	void Texture3D::OnInvalidate()
	{
		if (glIsTexture(m_texture))
		{
			glDeleteTextures(1, &m_texture);
			m_texture = -1;
		}
	}

	void Texture3D::Validate()
	{
		assert(glIsTexture(m_texture) == false);

		glGenTextures(1, &m_texture);
		
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, m_texture));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, m_magFilter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, m_minFilter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, m_wrapModeR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, m_wrapModeS));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, m_wrapModeT));
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, GL_NONE));
	}

	int Texture3D::Width()
	{
		return m_width;
	}

	void Texture3D::Width(int _value)
	{
		if (m_width == _value) return;
		m_width = _value;
		Invalidate();
	}

	int Texture3D::Height()
	{
		return m_height;
	}

	void Texture3D::Height(int _value)
	{
		if (m_depth == _value) return;
		m_depth = _value;
		Invalidate();
	}

	int Texture3D::Depth()
	{
		return m_depth;
	}

	void Texture3D::Depth(int _value)
	{
		if (m_depth == _value) return;
		m_depth = _value;
		Invalidate();
	}

	GLenum Texture3D::Format()
	{
		return m_format;
	}

	void Texture3D::Format(GLenum _value)
	{
		if (m_format == _value) return;
		m_format = _value;
		Invalidate();
	}

	GLenum Texture3D::MagFilter()
	{
		return m_magFilter;
	}

	void Texture3D::MagFilter(GLenum _value)
	{
		if (m_magFilter == _value) return;
		m_magFilter = _value;
		Invalidate();
	}

	GLuint Texture3D::MinFilter()
	{
		return m_minFilter;
	}

	void Texture3D::MinFilter(GLenum _value)
	{
		if (m_minFilter == _value) return;
		m_minFilter = _value;
		Invalidate();
	}

	GLenum Texture3D::WrapModeR()
	{
		return m_wrapModeR;
	}

	void Texture3D::WrapModeR(GLenum _value)
	{
		if (m_wrapModeR == _value)
			return;
		m_wrapModeR = _value;
		Invalidate();
	}

	GLenum Texture3D::WrapModeS()
	{
		return m_wrapModeS;
	}

	void Texture3D::WrapModeS(GLenum _value)
	{
		if (m_wrapModeS == _value)
			return;
		m_wrapModeS = _value;
		Invalidate();
	}

	GLenum Texture3D::WrapModeT()
	{
		return m_wrapModeT;
	}

	void Texture3D::WrapModeT(GLenum _value)
	{
		if (m_wrapModeT == _value)
			return;
		m_wrapModeT = _value;
		Invalidate();
	}

	GLuint Texture3D::TextureName()
	{
		ValidateNow();
		return m_texture;
	}
}