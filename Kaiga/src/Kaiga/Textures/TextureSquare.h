#pragma once

#include <Kaiga\Core\ITextureSquare.h>
#include <Kaiga\Core\TextureBase.h>

namespace Kaiga
{
	class TextureSquare : public ITextureSquare, public TextureBase
	{
	public:
		TextureSquare();
		~TextureSquare();

		int GetNumMipMaps();
	protected:
		// Inherited via AbstractValidatable
		virtual void Validate() override;
	};
}