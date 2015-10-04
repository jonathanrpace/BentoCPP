#pragma once

#include <Kaiga\Core\ITexture.h>

namespace Kaiga
{
	class ITextureRect : ITexture
	{
		virtual ~ITextureRect() = 0;
		virtual void SetSize(int _width, int _height) = 0;
		virtual int GetWidth() = 0;
		virtual int GetHeight() = 0;
	};
}