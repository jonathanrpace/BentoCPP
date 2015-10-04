#pragma once

#include <Kaiga/Core/ITexture.h>

namespace Kaiga
{
	class ITextureSquare : public ITexture
	{
	public:
		virtual ~ITextureSquare() = 0;
		virtual void SetSize(int _size) = 0;
		virtual int GetSize() = 0;
	};
}