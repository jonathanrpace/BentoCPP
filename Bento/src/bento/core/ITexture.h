#pragma once

#include <gl/glew.h>

namespace bento
{
	struct ITexture
	{
		virtual GLuint TextureName() = 0;
		virtual GLenum Target() = 0;
		virtual GLenum Format() = 0;
		virtual GLenum MinFilter() = 0;
		virtual void MinFilter(GLenum) = 0;
		virtual GLenum MagFilter() = 0;
		virtual void MagFilter(GLenum) = 0;
		virtual GLenum WrapModeS() = 0;
		virtual void WrapModeS(GLenum) = 0;
		virtual GLenum WrapModeT() = 0;
		virtual void WrapModeT(GLenum) = 0;
	};
}