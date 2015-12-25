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

		TextureSquare& HeightDataA() { return m_heightDataA; }
		TextureSquare& HeightDataB() { return m_heightDataB; }
		TextureSquare& FluxDataA() { return m_fluxDataA; }
		TextureSquare& FluxDataB() { return m_fluxDataB; }
		TextureSquare& MappingDataA() { return m_mappingDataA; }
		TextureSquare& MappingDataB() { return m_mappingDataB; }
		TextureSquare& VelocityData() { return m_velocityData; }
		TextureSquare& NormalData() { return m_normalData; }
		
		TextureSquare& HeightDataRead() { return *m_heightDataRead; }
		TextureSquare& HeightDataWrite() { return m_heightDataRead == &m_heightDataA ? m_heightDataB : m_heightDataA; }
		TextureSquare& FluxDataRead() { return *m_fluxDataRead; }
		TextureSquare& MappingDataRead() { return *m_mappingDataRead; }

		void SwapHeightData()
		{
			TextureSquare* tmp = m_heightDataRead;
			m_heightDataRead = m_heightDataWrite;
			m_heightDataWrite = tmp;
		}

		void SwapFluxData()
		{
			TextureSquare* tmp = m_fluxDataRead;
			m_fluxDataRead = m_fluxDataWrite;
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
		

		TextureSquare m_heightDataA;
		TextureSquare m_heightDataB;
		TextureSquare m_fluxDataA;
		TextureSquare m_fluxDataB;
		TextureSquare m_mappingDataA;
		TextureSquare m_mappingDataB;

		TextureSquare m_velocityData;
		TextureSquare m_normalData;
		
		TextureSquare* m_heightDataRead;
		TextureSquare* m_heightDataWrite;

		TextureSquare* m_fluxDataRead;
		TextureSquare* m_fluxDataWrite;

		TextureSquare* m_mappingDataRead;
		TextureSquare* m_mappingDataWrite;
	};
}