#include "TextureResource.h"

#include <string.h>
#include <stdlib.h>
#include <gli/gli.hpp>

#include <bento.h>

namespace bento
{
	TextureResource::TextureResource
	(
		char* const _filename, /* = nullptr */
		GLenum _magFilter, /* = GL_LINEAR */
		GLenum _minFilter, /* = GL_LINEAR_MIPMAP_LINEAR */ 
		GLenum _wrapModeR, /* = GL_REPEAT */
		GLenum _wrapModeS  /* = GL_REPEAT */
	)
		: TextureBase(-1, GL_RGBA8, _magFilter, _minFilter, _wrapModeR, _wrapModeS)
	{
		if ( _filename != nullptr )
		{
			Load(_filename);
		}
	}

	TextureResource::~TextureResource()
	{

	}

	int TextureResource::Load(char* const _filename)
	{
		size_t resolvedFilenameSize = strlen(_filename) + strlen(Config::ResourcePath().c_str()) + 1;
		char* resolvedFilename = new char[resolvedFilenameSize];
		strcpy_s(resolvedFilename, resolvedFilenameSize, Config::ResourcePath().c_str());
		strcat_s(resolvedFilename, resolvedFilenameSize, _filename);

		gli::texture Texture = gli::load(resolvedFilename);

		delete[] resolvedFilename;

		if(Texture.empty())
			return 0;

		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
		GLenum Target = GL.translate(Texture.target());
		m_target = GL.translate(Texture.target());
		m_internalFormat = Format.Internal;
		
		Invalidate();
		ValidateNow();

		GL_CHECK( glBindTexture(Target, m_texture) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1)) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]) );
		GL_CHECK( glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]) );
		
		glm::tvec3<GLsizei> const Extent(Texture.extent());
		GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());
		GLsizei const Levels = static_cast<GLsizei>(Texture.levels());

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			GL_CHECK( glTexStorage1D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x) );
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
			GL_CHECK( glTexStorage2D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal) );
			break;
		case gli::TARGET_CUBE:
			GL_CHECK( glTexStorage2D(
				GL_TEXTURE_CUBE_MAP, Levels, Format.Internal, Extent.x, Extent.x) );
			break;
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			GL_CHECK( glTexStorage3D(
				Target, static_cast<GLint>(Texture.levels()), Format.Internal,
				Extent.x, Extent.y,
				Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal) );
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
				{
					GL_CHECK( glCompressedTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Extent.x,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level)) );
				}
				else
				{
					GL_CHECK( glTexSubImage1D(
						Target, static_cast<GLint>(Level), 0, Extent.x,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level)) );
				}
				break;
			case gli::TARGET_1D_ARRAY:
			case gli::TARGET_2D:
			case gli::TARGET_CUBE:
				if(gli::is_compressed(Texture.format()))
				{
					GL_CHECK( glCompressedTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0,
						Extent.x,
						Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level)) );
				}
				else
				{
					GL_CHECK( glTexSubImage2D(
						Target, static_cast<GLint>(Level),
						0, 0,
						Extent.x,
						Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level)) );
				}
				break;
			case gli::TARGET_2D_ARRAY:
			case gli::TARGET_3D:
			case gli::TARGET_CUBE_ARRAY:
				if(gli::is_compressed(Texture.format()))
				{
					GL_CHECK( glCompressedTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0,
						Extent.x, Extent.y,
						Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
						Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
						Texture.data(Layer, Face, Level)) );
				}
				else
				{
					GL_CHECK( glTexSubImage3D(
						Target, static_cast<GLint>(Level),
						0, 0, 0,
						Extent.x, Extent.y,
						Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
						Format.External, Format.Type,
						Texture.data(Layer, Face, Level)) );
				}
				break;
			default: assert(0); break;
			}
		}
		return 1;
	}
}
