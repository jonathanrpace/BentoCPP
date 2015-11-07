#include "TerrainGeometry.h"

#include <random>

#include <bento.h>

namespace bento
{
	TerrainGeometry::TerrainGeometry(std::string _name)
		: GeometryBase(_name)
		, m_size(2.0f)
		, m_numVerticesPerDimension(512)
		, m_heightDataA(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_heightDataB(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_fluxDataA(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_fluxDataB(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_mappingDataA(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_mappingDataB(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_velocityData(m_numVerticesPerDimension, GL_RGBA16F, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT)
		, m_heightDataRead(&m_heightDataA)
		, m_heightDataWrite(&m_heightDataB)
		, m_fluxDataRead(&m_fluxDataA)
		, m_fluxDataWrite(&m_fluxDataB)
		, m_mappingDataRead(&m_mappingDataA)
		, m_mappingDataWrite(&m_mappingDataB)
	{
	}

	TerrainGeometry::~TerrainGeometry()
	{
	}

	void TerrainGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		glGenVertexArrays(1, &m_vertexArrayName);
		GL_CHECK(glBindVertexArray(m_vertexArrayName));

		
		SetVertexFormatf(0, 3);	// Position;
		SetVertexFormatf(1, 2);	// UV;

		glBindVertexArray(0);

		m_numVertices = m_numVerticesPerDimension * m_numVerticesPerDimension;
		int numTriangles = (m_numVerticesPerDimension - 1) * (m_numVerticesPerDimension - 1) * 2;
		m_numIndices = numTriangles * 3;

		std::vector<float> positions(m_numVertices * 3);
		std::vector<float> uvs(m_numVertices * 2);
		std::vector<int> indices(m_numIndices);

		std::vector<float> heightData(m_numVertices * 4);
		std::vector<float> fluxData(m_numVertices * 4);
		std::vector<float> velocityData(m_numVertices * 4);

		m_heightDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_heightDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_fluxDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_fluxDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_velocityData.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);

		std::srand(0);

		int indicesIndex = 0;
		int vertexIndex = 0;
		for (int i = 0; i < m_numVerticesPerDimension; i++)
		{
			float zRatio = (float)i / (m_numVerticesPerDimension - 1);
			for (int j = 0; j < m_numVerticesPerDimension; j++)
			{
				float xRatio = (float)j / (m_numVerticesPerDimension - 1);
				int float2Index = (i * m_numVerticesPerDimension + j) * 2;
				int float3Index = (i * m_numVerticesPerDimension + j) * 3;
				int float4Index = (i * m_numVerticesPerDimension + j) * 4;

				positions[float3Index + 0] = (xRatio * m_size) - m_size * 0.5f;
				positions[float3Index + 1] = 0.0f;
				positions[float3Index + 2] = (zRatio * m_size) - m_size * 0.5f;

				uvs[float2Index + 0] = xRatio;
				uvs[float2Index + 1] = zRatio;

				heightData[float4Index + 0] = 0.0f;// xRatio * 0.5f;// 0.0f;// static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);	// solidHeight
				heightData[float4Index + 1] = 0.001f;// 0.1f * (static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX));	// moltenHeight
				heightData[float4Index + 2] = 0.0f;	// empty
				heightData[float4Index + 3] = 0.0f;	// empty
			
				fluxData[float4Index + 0] = 0.0f;
				fluxData[float4Index + 1] = 0.0f;
				fluxData[float4Index + 2] = 0.0f;
				fluxData[float4Index + 3] = 0.0f;

				velocityData[float4Index + 0] = xRatio;	
				velocityData[float4Index + 1] = zRatio;	
				velocityData[float4Index + 2] = 0.0f;	
				velocityData[float4Index + 3] = 0.0f;	

				if (i < m_numVerticesPerDimension - 1 && j < m_numVerticesPerDimension - 1)
				{
					indices[indicesIndex] = vertexIndex;
					indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension;
					indices[indicesIndex + 2] = vertexIndex + 1;
					indicesIndex += 3;

					indices[indicesIndex] = vertexIndex + 1;
					indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension;
					indices[indicesIndex + 2] = vertexIndex + m_numVerticesPerDimension + 1;
					indicesIndex += 3;
				}

				vertexIndex++;
			}
		}

		m_heightDataA.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_heightDataB.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_fluxDataA.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);
		m_fluxDataB.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);

		m_mappingDataA.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);
		m_mappingDataB.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);

		m_velocityData.TexImage2D(GL_RGBA, GL_FLOAT, &velocityData[0]);
		
		BufferVertexData(0, &positions[0], m_numVertices * 3);
		BufferVertexData(1, &uvs[0], m_numVertices * 2);
		BufferIndexData(0, &indices[0], m_numIndices);
	}

	void TerrainGeometry::OnInvalidate()
	{
		GeometryBase::OnInvalidate();
	}

	const std::type_info & TerrainGeometry::TypeInfo()
	{
		return typeid(TerrainGeometry);
	}
}