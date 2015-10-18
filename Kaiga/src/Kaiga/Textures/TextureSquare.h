#pragma once

#include <kaiga.h>

namespace Kaiga
{
	class TextureSquare : public ITextureSquare, public TextureBase
	{
	public:
		TextureSquare();
		~TextureSquare();

		// Inherited via ITextureSquare
		virtual void SetSize(int _size) override;
		virtual int GetSize() override;

		int GetNumMipMaps();
	private:
		int m_size;
	protected:
		// Inherited via AbstractValidatable
		virtual void OnInvalidate() override;
		virtual void Validate() override;
	};
}