#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/IInspectable.h>
#include <bento/components/geom/Geometry.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/components/textures/PingPongTextureSquare.h>

using namespace bento;

namespace godBox
{
	class TerrainSidesGeometry
		: public Geometry
		, public SharedObject<TerrainSidesGeometry>
		, public IInspectable
	{
	public:
		TerrainSidesGeometry(std::string _name = "TerrainSidesGeometry");
		~TerrainSidesGeometry();

		// From IInspectable
		virtual void AddUIElements() override;

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		float m_size;
		float m_height;
		int m_numVerticesPerDimension;
	};
}