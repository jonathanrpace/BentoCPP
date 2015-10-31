#pragma once

#include <bento/core/ITextureSquare.h>
#include <bento/core/TextureBase.h>

namespace bento
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