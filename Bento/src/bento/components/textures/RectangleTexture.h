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

		void TexImage2D(GLenum _format, GLenum _type, const GLvoid * _data);

		void Width(int _width);
		void Height(int _height);
		inline int Width() { return m_width; }
		inline int Height() { return m_height; }

	protected:
		// From AbstractValidatable
		virtual void Validate() override;

		int m_width;
		int m_height;
	};
}