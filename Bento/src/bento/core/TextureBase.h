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
			GLenum _internalFormat = GL_RGBA8,
			GLenum _magFilter = GL_LINEAR,
			GLenum _minFilter = GL_LINEAR_MIPMAP_LINEAR,
			GLenum _wrapModeS = GL_REPEAT,
			GLenum _wrapModeT = GL_REPEAT
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
		virtual GLenum WrapModeT() override;
		virtual void WrapModeT(GLenum _value) override;
		virtual GLenum InternalFormat() override;

	protected:
		GLenum m_target;
		GLenum m_internalFormat;
		GLuint m_texture;
		GLenum m_minFilter;
		GLenum m_magFilter;
		GLenum m_wrapModeS;
		GLenum m_wrapModeT;
	};
}