#pragma once

#include <glew.h>
#include <Kaiga\Core\ITexture2D.h>
#include <Ramen\Core\AbstractValidatable.h>

namespace Kaiga
{
	class TextureBase 
		: public ITexture2D
		, public Ramen::AbstractValidatable
	{
	public:
		TextureBase(
			int _width = 256,
			int _height = 256,
			GLuint _format = GL_RGBA,
			GLuint _magFilter = GL_LINEAR,
			GLuint _minFilter = GL_LINEAR
		);
		~TextureBase();

		// AbstractValidatable
		virtual void OnInvalidate() override;
		virtual void Validate() = 0;

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
		GLuint m_texture;
		int m_width;
		int m_height;
		GLuint m_name;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeS;
		GLenum m_wrapModeR;
		GLenum m_format;
	};
}