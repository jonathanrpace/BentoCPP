#pragma once

#include <bento/core/TextureBase.h>

namespace bento
{
	class RectangleTexture 
		: public TextureBase
	{
	public:
		RectangleTexture(
			int _width = 256,
			int _height = 256,
			GLuint _format = GL_RGBA16F,
			GLuint _magFilter = GL_LINEAR,
			GLuint _minFilter = GL_LINEAR
		);

		// From AbstractValidatable
		virtual void Validate() override;
	};
}