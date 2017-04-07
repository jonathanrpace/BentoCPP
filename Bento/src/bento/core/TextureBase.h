#pragma once

#include <gl/glew.h>

#include <bento/core/ITexture.h>
#include <bento/core/AbstractValidatable.h>

namespace bento
{
	class TextureBase 
		: public ITexture
		, public AbstractValidatable
	{
	public:
		TextureBase
		(
			GLenum _target = -1,
			GLenum _format = GL_RGBA8,
			GLenum _magFilter = GL_LINEAR,
			GLenum _minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum _wrapModeR = GL_REPEAT,
			GLenum _wrapModeS = GL_REPEAT
		);
		~TextureBase();

		// AbstractValidatable
		virtual void OnInvalidate() override;
		virtual void Validate();

		// ITexture
		virtual GLuint TextureName() override;
		virtual GLenum Target() override;
		virtual GLenum MinFilter() override;
		virtual void MinFilter(GLenum _value) override;
		virtual GLenum MagFilter() override;
		virtual void MagFilter(GLenum _value) override;
		virtual GLenum WrapModeS() override;
		virtual void WrapModeS(GLenum _value) override;
		virtual GLenum WrapModeR() override;
		virtual void WrapModeR(GLenum _value) override;
		virtual GLenum Format() override;

	protected:
		GLenum m_target;
		GLenum m_format;
		GLuint m_texture;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeS;
		GLenum m_wrapModeR;
	};
}