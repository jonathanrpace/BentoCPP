#pragma once

#include <glew.h>

namespace Kaiga
{
	struct ITexture
	{
		virtual GLuint TextureName() = 0;
	};
}