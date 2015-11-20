#pragma once

#include <gl/glew.h>

namespace bento
{
	struct ITexture
	{
		virtual GLuint TextureName() = 0;
	};
}