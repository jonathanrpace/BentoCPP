#pragma once

#include <Kaiga\Core\ITextureSquare.h>
#include <Kaiga\Textures\AbstractTexture.h>
#include <Kaiga\Util\TextureUtil.h>

namespace Kaiga
{
	class TextureSquare : public ITextureSquare, public AbstractTexture
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