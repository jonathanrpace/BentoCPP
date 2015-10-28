#include "RectangleTexture.h"

#include <glew.h>
#include <Kaiga/Util/GLErrorUtil.h>

Kaiga::RectangleTexture::RectangleTexture(
	int _width, 
	int _height, 
	GLuint _format, 
	GLuint _magFilter, 
	GLuint _minFilter
) 
	: TextureBase(GL_TEXTURE_RECTANGLE, _width, _height, _format, _magFilter, _minFilter)
{

}

void Kaiga::RectangleTexture::Validate()
{
	GL_CHECK(glGenTextures(1, &m_texture));
	GL_CHECK(glBindTexture(GL_TEXTURE_RECTANGLE, m_texture));
	GL_CHECK(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, m_format, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GL_CHECK(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, m_magFilter));
	GL_CHECK(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, m_minFilter));
	GL_CHECK(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
	GL_CHECK(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CHECK(glBindTexture(GL_TEXTURE_RECTANGLE, 0));
}
