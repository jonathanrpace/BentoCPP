#pragma once

#include <glew.h>
#include <bento/core/ITexture.h>

namespace bento
{
	struct ITexture2D : ITexture
	{
		virtual void SetSize(int _width, int _height) = 0;
		virtual int Width() = 0;
		virtual void Width(int) = 0;
		virtual int Height() = 0;
		virtual void Height(int) = 0;
		virtual GLenum MinFilter() = 0;
		virtual void MinFilter(GLenum) = 0;
		virtual GLenum MagFilter() = 0;
		virtual void MagFilter(GLenum) = 0;
		virtual GLenum WrapModeR() = 0;
		virtual void WrapModeR(GLenum) = 0;
		virtual GLenum WrapModeS() = 0;
		virtual void WrapModeS(GLenum) = 0;
		virtual GLenum Format() = 0;
		virtual void Format(GLenum) = 0;
	};
}