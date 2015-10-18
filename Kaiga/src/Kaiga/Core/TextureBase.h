#pragma once

#include <glew.h>
#include <Kaiga\Core\ITexture.h>
#include <Ramen\Core\AbstractValidatable.h>

namespace Kaiga
{
	class TextureBase : public ITexture, public Ramen::AbstractValidatable
	{
	public:
		TextureBase();
		~TextureBase();

		// Inherited via ITexture
		virtual GLuint GetName() override;
		virtual GLenum GetMinFilter() override;
		virtual void SetMinFilter(GLenum _value) override;
		virtual GLenum GetMagFilter() override;
		virtual void SetMagFilter(GLenum _value) override;
		virtual GLenum GetWrapModeS() override;
		virtual void SetWrapModeS(GLenum _value) override;
		virtual GLenum GetWrapModeR() override;
		virtual void SetWrapModeR(GLenum _value) override;
		virtual GLenum GetPixelInternalFormat() override;
		virtual void SetPixelInternalFormat(GLenum _value) override;

	protected:
		GLuint m_name;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeS;
		GLenum m_wrapModeR;
		GLenum m_pixelInternalFormat;
	};
}