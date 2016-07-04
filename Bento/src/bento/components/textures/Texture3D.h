#pragma once

#include <gl/glew.h>

#include <bento/core/AbstractValidatable.h>

namespace bento
{
	class Texture3D 
		: public bento::AbstractValidatable
	{
	public:
		Texture3D(
			int _width = 256,
			int _height = 256,
			int _depth = 256,
			GLenum _format = GL_RGBA16F,
			GLenum _magFilter = GL_LINEAR,
			GLenum _minFilter = GL_LINEAR,
			GLenum _wrapModeR = GL_CLAMP,
			GLenum _wrapModeS = GL_CLAMP,
			GLenum _wrapModeT = GL_CLAMP
		);
		~Texture3D();

		void TexImage3D(GLenum _format, GLenum _type, const GLvoid* _data, int _level = 0);
		void TexImage3D(const char* _filename);

		// AbstractValidatable
		virtual void OnInvalidate() override;
		virtual void Validate();

		GLuint TextureName();

		int Width();
		void Width(int);

		int Height();
		void Height(int);

		int Depth();
		void Depth(int);

		GLenum MinFilter();
		void MinFilter(GLenum _value);

		GLenum MagFilter();
		void MagFilter(GLenum _value);

		GLenum WrapModeR();
		void WrapModeR(GLenum _value);

		GLenum WrapModeS();
		void WrapModeS(GLenum _value);

		GLenum WrapModeT();
		void WrapModeT(GLenum _value);

		GLenum Format();
		void Format(GLenum _value);

	protected:
		GLuint m_texture;
		int m_width;
		int m_height;
		int m_depth;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeR;
		GLenum m_wrapModeS;
		GLenum m_wrapModeT;
		GLenum m_format;
	};
}