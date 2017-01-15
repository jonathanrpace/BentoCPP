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

	int TextureBase::Load(const char* _filename)
	{
		ValidateNow();

		size_t resolvedFilenameSize = strlen(_filename) + strlen(bento::Config::ResourcePath().c_str()) + 1;
		char* resolvedFilename = new char[resolvedFilenameSize];
		strcpy_s(resolvedFilename, resolvedFilenameSize, bento::Config::ResourcePath().c_str());
		strcat_s(resolvedFilename, resolvedFilenameSize, _filename);

		gli::texture Texture = gli::load(resolvedFilename);

		delete[] resolvedFilename;

		if(Texture.empty())
			return 0;

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());

		glBindTexture(Target, m_texture);
		glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
		glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

		glm::tvec3<GLsizei> const Extent(Texture.extent());
		GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

		m_width = Extent.x;
		m_height = Extent.y;

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			glTexStorage1D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			glTexStorage2D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			glTexStorage3D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Extent.x, Extent.y,
				Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
			break;
		default:
			assert(0);
			break;
		}

		for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
		for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
		for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
		{
			GLsizei const LayerGL = static_cast<GLsizei>(Layer);
			glm::tvec3<GLsizei> Extent(Texture.extent(Level));
			Target = gli::is_target_cube(Texture.target())
				? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
				: Target;

			switch(Texture.target())
			{
			case gli::TARGET_1D:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Extent.x,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level));
				else
					glTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Extent.x,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level));
				break;
			case gli::TARGET_1D_ARRAY:
			case gli::TARGET_2D:
			case gli::TARGET_CUBE:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0,
						Extent.x,
						Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level));
				else
					glTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0,
						Extent.x,
						Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level));
				break;
			case gli::TARGET_2D_ARRAY:
			case gli::TARGET_3D:
			case gli::TARGET_CUBE_ARRAY:
				if(gli::is_compressed(Texture.format()))
					glCompressedTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0,
						Extent.x, Extent.y,
						Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level));
				else
					glTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0,
						Extent.x, Extent.y,
						Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level));
				break;
			default: assert(0); break;
			}
		}
		return 1;
	}

	void TextureBase::TexImage2D(GLenum _format, GLenum _type, const GLvoid * _data, int _level) /* _level = 0 */
	{
		ValidateNow();
		GL_CHECK(glBindTexture(m_textureTarget, m_texture));
		GL_CHECK(glTexImage2D(m_textureTarget, _level, m_format, m_width, m_height, 0, _format, _type, _data));
	}

	void TextureBase::TexImage2D(const char* _filename) /* _level = 0 */
	{
		size_t resolvedFilenameSize = strlen(_filename) + strlen(bento::Config::ResourcePath().c_str()) + 1;
		char* resolvedFilename = new char[resolvedFilenameSize];
		strcpy_s(resolvedFilename, resolvedFilenameSize, bento::Config::ResourcePath().c_str());
		strcat_s(resolvedFilename, resolvedFilenameSize, _filename);
		
		int x, y, n;
		unsigned char *data = stbi_load(resolvedFilename, &x, &y, &n, 4);
		m_width = x;
		m_height = y;
		m_format = GL_RGBA8;

		TexImage2D(GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
		delete[] resolvedFilename;
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