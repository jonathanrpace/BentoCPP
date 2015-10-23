#pragma once

#include <kaiga.h>

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
		virtual void OnInvalidate() override;
		virtual void Validate() override;
	};
}