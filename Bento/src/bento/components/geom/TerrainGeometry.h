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

		// TODO: Make 'PingPongTexture' that wraps up two duplicate textures
		TextureSquare& HeightDataRead() { return *m_heightDataRead; }
		TextureSquare& HeightDataWrite() { return m_heightDataRead == &m_heightDataA ? m_heightDataB : m_heightDataA; }

		TextureSquare& VelocityDataRead() { return *m_velocityDataRead; }
		TextureSquare& VelocityDataWrite() { return m_velocityDataRead == &m_velocityDataA ? m_velocityDataB : m_velocityDataA; }

		TextureSquare& MiscDataRead() { return *m_miscDataRead; }
		TextureSquare& MiscDataWrite() { return m_miscDataRead == &m_miscDataA ? m_miscDataB : m_miscDataA; }

		TextureSquare& NormalDataRead() { return *m_normalDataRead; }
		TextureSquare& NormalDataWrite() { return m_normalDataRead == &m_normalDataA ? m_normalDataB : m_normalDataA; }

		TextureSquare& MoltenMapDataRead() { return *m_moltenMapDataRead; }
		TextureSquare& MoltenMapDataWrite() { return m_moltenMapDataRead == &m_moltenMapDataA ? m_moltenMapDataB : m_moltenMapDataA; }

		TextureSquare& SmudgeDataRead() { return *m_smudgeDataRead; }
		TextureSquare& SmudgeDataWrite() { return m_smudgeDataRead == &m_smudgeDataA ? m_smudgeDataB : m_smudgeDataA; }

		TextureSquare& RockFluxDataRead() { return *m_rockFluxDataRead; }
		TextureSquare& RockFluxDataWrite() { return m_rockFluxDataRead == &m_rockFluxDataA ? m_rockFluxDataB : m_rockFluxDataA; }

		TextureSquare& WaterFluxDataRead() { return *m_waterFluxDataRead; }
		TextureSquare& WaterFluxDataWrite() { return m_waterFluxDataRead == &m_waterFluxDataA ? m_waterFluxDataB : m_waterFluxDataA; }
		
		void SwapHeightData()
		{
			TextureSquare* tmp = m_heightDataRead;
			m_heightDataRead = m_heightDataWrite;
			m_heightDataWrite = tmp;
		}

		void SwapVelocityData()
		{
			TextureSquare* tmp = m_velocityDataRead;
			m_velocityDataRead = m_velocityDataWrite;
			m_velocityDataWrite = tmp;
		}

		void SwapMiscData()
		{
			TextureSquare* tmp = m_miscDataRead;
			m_miscDataRead = m_miscDataWrite;
			m_miscDataWrite = tmp;
		}

		void SwapNormalData()
		{
			TextureSquare* tmp = m_normalDataRead;
			m_normalDataRead = m_normalDataWrite;
			m_normalDataWrite = tmp;
		}

		void SwapMoltenMapData()
		{
			TextureSquare* tmp = m_moltenMapDataRead;
			m_moltenMapDataRead = m_moltenMapDataWrite;
			m_moltenMapDataWrite = tmp;
		}

		void SwapSmudgeData()
		{
			TextureSquare* tmp = m_smudgeDataRead;
			m_smudgeDataRead = m_smudgeDataWrite;
			m_smudgeDataWrite = tmp;
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
		TextureSquare m_velocityDataA;
		TextureSquare m_velocityDataB;
		TextureSquare m_miscDataA;
		TextureSquare m_miscDataB;
		TextureSquare m_normalDataA;
		TextureSquare m_normalDataB;
		TextureSquare m_moltenMapDataA;
		TextureSquare m_moltenMapDataB;
		TextureSquare m_smudgeDataA;
		TextureSquare m_smudgeDataB;
		TextureSquare m_rockFluxDataA;
		TextureSquare m_rockFluxDataB;
		TextureSquare m_waterFluxDataA;
		TextureSquare m_waterFluxDataB;

		TextureSquare* m_heightDataRead;
		TextureSquare* m_heightDataWrite;
		TextureSquare* m_velocityDataRead;
		TextureSquare* m_velocityDataWrite;
		TextureSquare* m_miscDataRead;
		TextureSquare* m_miscDataWrite;
		TextureSquare* m_normalDataRead;
		TextureSquare* m_normalDataWrite;
		TextureSquare* m_moltenMapDataRead;
		TextureSquare* m_moltenMapDataWrite;
		TextureSquare* m_smudgeDataRead;
		TextureSquare* m_smudgeDataWrite;
		TextureSquare* m_rockFluxDataRead;
		TextureSquare* m_rockFluxDataWrite;
		TextureSquare* m_waterFluxDataRead;
		TextureSquare* m_waterFluxDataWrite;
	};
}