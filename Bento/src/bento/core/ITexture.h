#pragma once

#include <glew.h>

namespace bento
{
	struct ITexture
	{
		virtual GLuint TextureName() = 0;
	};
}