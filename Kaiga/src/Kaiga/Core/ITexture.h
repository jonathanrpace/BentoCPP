#pragma once

#include <glew.h>

namespace Kaiga
{
	class ITexture
	{
	public:
		virtual ~ITexture() = 0;
		virtual GLuint GetName() = 0;
		virtual GLenum GetMinFilter() = 0;
		virtual void SetMinFilter(GLenum) = 0;
		virtual GLenum GetMagFilter() = 0;
		virtual void SetMagFilter(GLenum) = 0;
		virtual GLenum GetWrapModeS() = 0;
		virtual void SetWrapModeS(GLenum) = 0;
		virtual GLenum GetWrapModeR() = 0;
		virtual void SetWrapModeR(GLenum) = 0;
		virtual GLenum GetPixelInternalFormat() = 0;
		virtual void SetPixelInternalFormat(GLenum) = 0;
	};
}