#pragma once

#include <bento/core/TextureBase.h>

namespace bento
{
	class TextureResource 
		: public TextureBase
	{
	public:
		TextureResource
		(
			char* const _filename = nullptr,
			GLenum _magFilter = GL_LINEAR,
			GLenum _minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum _wrapModeR = GL_REPEAT,
			GLenum _wrapModeS = GL_REPEAT
		);
		~TextureResource();

		int Load(char* const _filename);
	};
}