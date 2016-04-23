#pragma once

#include <bento/core/SharedObject.h>
#include <bento/core/IInspectable.h>
#include <bento/components/geom/Geometry.h>
#include <bento/components/textures/TextureSquare.h>

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

		TextureSquare& RockDataRead() { return *m_rockDataRead; }
		TextureSquare& RockDataWrite() { return m_rockDataRead == &m_rockDataA ? m_rockDataB : m_rockDataA; }

		TextureSquare& RockFluxDataRead() { return *m_rockFluxDataRead; }
		TextureSquare& RockFluxDataWrite() { return m_rockFluxDataRead == &m_rockFluxDataA ? m_rockFluxDataB : m_rockFluxDataA; }

		TextureSquare& WaterDataRead() { return *m_waterDataRead; }
		TextureSquare& WaterDataWrite() { return m_waterDataRead == &m_waterDataA ? m_waterDataB : m_waterDataA; }

		TextureSquare& WaterFluxDataRead() { return *m_waterFluxDataRead; }
		TextureSquare& WaterFluxDataWrite() { return m_waterFluxDataRead == &m_waterFluxDataA ? m_waterFluxDataB : m_waterFluxDataA; }
		
		TextureSquare& MappingDataRead() { return *m_mappingDataRead; }
		TextureSquare& MappingDataWrite() { return m_mappingDataRead == &m_mappingDataA ? m_mappingDataB : m_mappingDataA; }

		TextureSquare& RockNormalData() { return m_rockNormalData; }
		TextureSquare& WaterNormalData() { return m_waterNormalData; }
		TextureSquare& WaterFoamData() { return m_waterFoamData; }

		void SwapRockData()
		{
			TextureSquare* tmp = m_rockDataRead;
			m_rockDataRead = m_rockDataWrite;
			m_rockDataWrite = tmp;
		}

		void SwapWaterData()
		{
			TextureSquare* tmp = m_waterDataRead;
			m_waterDataRead = m_waterDataWrite;
			m_waterDataWrite = tmp;
		}

		void SwapRockFluxData()
		{
			TextureSquare* tmp = m_rockFluxDataRead;
			m_rockFluxDataRead = m_rockFluxDataWrite;
			m_rockFluxDataWrite = tmp;
		}

		void SwapWaterFluxData()
		{
			TextureSquare* tmp = m_waterFluxDataRead;
			m_waterFluxDataRead = m_waterFluxDataWrite;
			m_waterFluxDataWrite = tmp;
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
		TextureSquare m_rockNormalData;

		TextureSquare* m_rockDataRead;
		TextureSquare* m_rockDataWrite;
		TextureSquare* m_rockFluxDataRead;
		TextureSquare* m_rockFluxDataWrite;

		TextureSquare m_waterDataA;
		TextureSquare m_waterDataB;
		TextureSquare m_waterFluxDataA;
		TextureSquare m_waterFluxDataB;
		TextureSquare m_waterNormalData;
		TextureSquare m_waterFoamData;
						
		TextureSquare* m_waterDataRead;
		TextureSquare* m_waterDataWrite;
		TextureSquare* m_waterFluxDataRead;
		TextureSquare* m_waterFluxDataWrite;

		TextureSquare m_mappingDataA;
		TextureSquare m_mappingDataB;

		TextureSquare* m_mappingDataRead;
		TextureSquare* m_mappingDataWrite;
	};
}