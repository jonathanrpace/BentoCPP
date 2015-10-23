#pragma once

#include <Kaiga\Core\ITexture2D.h>

namespace Kaiga
{
	struct ITextureRect : ITexture2D
	{
		virtual ~ITextureRect() = 0;
	};
}