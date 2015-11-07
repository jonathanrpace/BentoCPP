#pragma once

#include <glew.h>

#include <bento/core/ITexture2D.h>
#include <bento/core/AbstractValidatable.h>

namespace bento
{
	class TextureBase 
		: public ITexture2D
		, public bento::AbstractValidatable
	{
	public:
		TextureBase(
			GLenum _textureTarget,
			int _width = 256,
			int _height = 256,
			GLenum _format = GL_RGBA16F,
			GLenum _magFilter = GL_NEAREST,
			GLenum _minFilter = GL_NEAREST,
			GLenum _wrapModeR = GL_REPEAT,
			GLenum _wrapModeS = GL_REPEAT
		);
		~TextureBase();

		void TexImage2D(GLenum _format, GLenum _type, const GLvoid* _data, int _level = 0);
		void TexImage2D(const char* _filename);

		void GetTexImage(GLint _level, GLenum _format, GLenum _type, GLvoid* _pixels);

		// AbstractValidatable
		virtual void OnInvalidate() override;
		virtual void Validate();

		// ITexture
		virtual GLuint TextureName() override;

		// ITexture2D
		virtual void SetSize(int _width, int _height);
		virtual int Width() override;
		virtual void Width(int) override;
		virtual int Height() override;
		virtual void Height(int) override;
		virtual GLenum MinFilter() override;
		virtual void MinFilter(GLenum _value) override;
		virtual GLenum MagFilter() override;
		virtual void MagFilter(GLenum _value) override;
		virtual GLenum WrapModeS() override;
		virtual void WrapModeS(GLenum _value) override;
		virtual GLenum WrapModeR() override;
		virtual void WrapModeR(GLenum _value) override;
		virtual GLenum Format() override;
		virtual void Format(GLenum _value) override;

	protected:
		GLenum m_textureTarget;
		GLuint m_texture;
		int m_width;
		int m_height;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeS;
		GLenum m_wrapModeR;
		GLenum m_format;
	};
}