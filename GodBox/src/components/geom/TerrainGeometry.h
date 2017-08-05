#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/IInspectable.h>
#include <bento/components/geom/Geometry.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/components/textures/PingPongTextureSquare.h>

using namespace bento;

namespace godBox
{
	struct TerrainMousePos
	{
		int z;
		int u;
		int v;
	};

	class TerrainGeometry
		: public Geometry
		, public SharedObject<TerrainGeometry>
		, public IInspectable
	{
	public:
		TerrainGeometry(std::string _name = "TerrainGeometry");
		~TerrainGeometry();

		PingPongTextureSquare& HeightData() { return m_heightData; }
		PingPongTextureSquare& MiscData() { return m_miscData; }
		PingPongTextureSquare& SmudgeData() { return m_smudgeData; }
		PingPongTextureSquare& MoltenFluxData() { return m_moltenFluxData; }
		PingPongTextureSquare& WaterFluxData() { return m_waterFluxData; }
		PingPongTextureSquare& UVOffsetData() { return m_uvOffsetData; }
		PingPongTextureSquare& PressureData() { return m_pressureData; }

		TextureSquare& DerivedData() { return m_derivedData; }
		TextureSquare& NormalData() { return m_normalData; }
		TextureSquare& DivergenceData() { return m_divergenceData; }
		
		void ResetTerrainMousePos();

		inline TerrainMousePos GetTerrainMousePos() const
		{
			return m_terrainMousePos;
		}

		inline GLuint MousePositionBuffer() const
		{
			return m_mousePositionBuffer;
		}

		inline int NumVerticesPerDimension() const
		{ 
			return m_numVerticesPerDimension;
		}

		inline float Size() const
		{ 
			return m_size; 
		}

		// From IInspectable
		virtual void AddUIElements() override;

		
	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		float m_size;
		int m_numVerticesPerDimension;

		GLuint m_mousePositionBuffer;
		TerrainMousePos m_terrainMousePos;

		PingPongTextureSquare m_heightData;
		PingPongTextureSquare m_velocityData;
		PingPongTextureSquare m_miscData;
		PingPongTextureSquare m_smudgeData;
		PingPongTextureSquare m_uvOffsetData;
		PingPongTextureSquare m_moltenFluxData;
		PingPongTextureSquare m_waterFluxData;
		PingPongTextureSquare m_pressureData;

		// Derived
		TextureSquare m_normalData;
		TextureSquare m_derivedData;
		TextureSquare m_divergenceData;
	};
}