#pragma once

#include <bento/core/TextureBase.h>

namespace bento
{
	class TextureSquare 
		: public TextureBase
	{
	public:
		TextureSquare
		(
			int _size = 256
			, GLenum _internalFormat = GL_RGBA8
			, GLenum _format = GL_RGBA
			, GLenum _type = GL_FLOAT
			, GLenum _magFilter = GL_LINEAR
			, GLenum _minFilter = GL_LINEAR
			, GLenum _wrapModeS = GL_REPEAT
			, GLenum _wrapModeT = GL_REPEAT
		);
		~TextureSquare();

		virtual void GenerateMipMaps();
		int GetNumMipMaps();
		void TexImage2D(const GLvoid * _data, int _level = 0);

		void Size(int _size);
		inline int Size() { return m_size; }

	protected:
		// Inherited via AbstractValidatable
		virtual void Validate() override;

		int m_size;
		GLenum m_format;
		GLenum m_type;
	};
}