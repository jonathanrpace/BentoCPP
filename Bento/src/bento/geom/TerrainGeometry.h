#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/GeometryBase.h>
#include <bento/textures/TextureSquare.h>

namespace bento
{
	class TerrainGeometry
		: public GeometryBase
		, public bento::SharedObject<TerrainGeometry>
	{
	public:
		TerrainGeometry();
		~TerrainGeometry();

		TextureSquare& ReadTexture0();
		TextureSquare& WriteTexture0();
		TextureSquare& ReadTexture1();
		TextureSquare& WriteTexture1();
		TextureSquare& ReadTexture2();
		TextureSquare& WriteTexture2();

		// ISceneObject
		virtual const std::type_info& typeInfo() override;

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		float m_size;
		int m_numVerticesPerDimension;

		TextureSquare m_texture0A;
		TextureSquare m_texture0B;
		bool m_texture0AIsWrite;
		TextureSquare m_texture1A;
		TextureSquare m_texture1B;
		bool m_texture1AIsWrite;
		TextureSquare m_texture2A;
		TextureSquare m_texture2B;
		bool m_texture2AIsWrite;
	};
}