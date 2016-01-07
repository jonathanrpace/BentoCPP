#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/IInspectable.h>
#include <bento/geom/Geometry.h>
#include <bento/textures/TextureSquare.h>

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

		TextureSquare& RockDataA() { return m_rockDataA; }
		TextureSquare& RockDataB() { return m_rockDataB; }
		TextureSquare& RockFluxDataA() { return m_rockFluxDataA; }
		TextureSquare& RockFluxDataB() { return m_rockFluxDataB; }
		TextureSquare& MappingDataA() { return m_mappingDataA; }
		TextureSquare& MappingDataB() { return m_mappingDataB; }
		TextureSquare& RockNormalData() { return m_rockNormalData; }
		
		TextureSquare& RockDataRead() { return *m_rockDataRead; }
		TextureSquare& RockDataWrite() { return m_rockDataRead == &m_rockDataA ? m_rockDataB : m_rockDataA; }
		TextureSquare& RockFluxDataRead() { return *m_rockFluxDataRead; }
		TextureSquare& MappingDataRead() { return *m_mappingDataRead; }

		void SwapRockData()
		{
			TextureSquare* tmp = m_rockDataRead;
			m_rockDataRead = m_heightDataWrite;
			m_heightDataWrite = tmp;
		}

		void SwapRockFluxData()
		{
			TextureSquare* tmp = m_rockFluxDataRead;
			m_rockFluxDataRead = m_fluxDataWrite;
			m_fluxDataWrite = tmp;
		}

		void SwapMappingData()
		{
			TextureSquare* tmp = m_mappingDataRead;
			m_mappingDataRead = m_mappingDataWrite;
			m_mappingDataWrite = tmp;
		}

		void ResetTerrainMousePos();

		inline TerrainMousePos GetTerrainMousePos()
		{
			return m_terrainMousePos;
		}

		inline GLuint MousePositionBuffer()
		{
			return m_mousePositionBuffer;
		}

		inline int NumVerticesPerDimension() 
		{ 
			return m_numVerticesPerDimension;
		}

		inline float Size() 
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
		

		TextureSquare m_rockDataA;
		TextureSquare m_rockDataB;
		TextureSquare m_rockFluxDataA;
		TextureSquare m_rockFluxDataB;
		TextureSquare m_mappingDataA;
		TextureSquare m_mappingDataB;

		TextureSquare m_rockNormalData;
		
		TextureSquare* m_rockDataRead;
		TextureSquare* m_heightDataWrite;

		TextureSquare* m_rockFluxDataRead;
		TextureSquare* m_fluxDataWrite;

		TextureSquare* m_mappingDataRead;
		TextureSquare* m_mappingDataWrite;
	};
}