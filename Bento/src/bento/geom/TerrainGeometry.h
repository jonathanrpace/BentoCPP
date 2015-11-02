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
		TerrainGeometry(std::string _name = "TerrainGeometry");
		~TerrainGeometry();

		TextureSquare& Texture0A() { return m_texture0A; }
		TextureSquare& Texture0B() { return m_texture0B; }
		TextureSquare& Texture1A() { return m_texture1A; }
		TextureSquare& Texture1B() { return m_texture1B; }
		TextureSquare& Texture2A() { return m_texture2A; }
		TextureSquare& Texture2B() { return m_texture2B; }

		// ISceneObject
		virtual const std::type_info& TypeInfo() override;

		inline int NumVerticesPerDimension() { return m_numVerticesPerDimension;  }

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		float m_size;
		int m_numVerticesPerDimension;

		TextureSquare m_texture0A;
		TextureSquare m_texture0B;
		TextureSquare m_texture1A;
		TextureSquare m_texture1B;
		TextureSquare m_texture2A;
		TextureSquare m_texture2B;
	};
}