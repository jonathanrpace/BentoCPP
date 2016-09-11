#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/IInspectable.h>
#include <bento/components/geom/Geometry.h>
#include <bento/components/textures/TextureSquare.h>
#include <bento/components/textures/PingPongTextureSquare.h>

namespace bento
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
		PingPongTextureSquare& VelocityData() { return m_velocityData; }
		PingPongTextureSquare& MiscData() { return m_miscData; }
		PingPongTextureSquare& NormalData() { return m_normalData; }
		PingPongTextureSquare& MoltenMapData() { return m_moltenMapData; }
		PingPongTextureSquare& SmudgeData() { return m_smudgeData; }
		PingPongTextureSquare& RockFluxData() { return m_rockFluxData; }
		PingPongTextureSquare& WaterFluxData() { return m_waterFluxData; }
		
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

		inline int MoltenMapResScalar() const 
		{ 
			return m_moltenMapResScalar; 
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
		int m_moltenMapResScalar;

		GLuint m_mousePositionBuffer;
		TerrainMousePos m_terrainMousePos;

		PingPongTextureSquare m_heightData;
		PingPongTextureSquare m_velocityData;
		PingPongTextureSquare m_miscData;
		PingPongTextureSquare m_normalData;
		PingPongTextureSquare m_moltenMapData;
		PingPongTextureSquare m_smudgeData;
		PingPongTextureSquare m_rockFluxData;
		PingPongTextureSquare m_waterFluxData;
	};
}