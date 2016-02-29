#include "TerrainGeometry.h"

#include <random>

#include <bento.h>
#include <imgui.h>

#include <bento/util/TextureUtil.h>

namespace bento
{
	TerrainGeometry::TerrainGeometry(std::string _name)
		: Geometry(_name, typeid(TerrainGeometry))
		, m_size(1.5f)
		, m_numVerticesPerDimension(1024)
		, m_rockDataA(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_rockDataB(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_rockFluxDataA(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_rockFluxDataB(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_rockNormalData(m_numVerticesPerDimension, GL_RGBA32F, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP)
		, m_rockDataRead(&m_rockDataA)
		, m_rockDataWrite(&m_rockDataB)
		, m_rockFluxDataRead(&m_rockFluxDataA)
		, m_rockFluxDataWrite(&m_rockFluxDataB)

		, m_waterDataA(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterDataB(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterFluxDataA(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterFluxDataB(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterNormalData(m_numVerticesPerDimension, GL_RGBA32F, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP)
		, m_waterFoamData(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterDataRead(&m_waterDataA)
		, m_waterDataWrite(&m_waterDataB)
		, m_waterFluxDataRead(&m_waterFluxDataA)
		, m_waterFluxDataWrite(&m_waterFluxDataB)

		, m_mappingDataA(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_mappingDataB(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_mappingDataRead(&m_mappingDataA)
		, m_mappingDataWrite(&m_mappingDataB)
		, m_terrainMousePos()
	{
		glGenBuffers(1, &m_mousePositionBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mousePositionBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_terrainMousePos), nullptr, GL_DYNAMIC_COPY);
	}

	TerrainGeometry::~TerrainGeometry()
	{
	}

	void TerrainGeometry::ResetTerrainMousePos()
	{
		m_terrainMousePos.z = INT_MAX;
	}

	void TerrainGeometry::AddUIElements()
	{
		if (ImGui::SliderFloat("Size", &m_size, 0.1f, 10.0f))
		{
			Invalidate();
		}
		if (ImGui::SliderInt("NumVerticesPerDimension", &m_numVerticesPerDimension, 4, 2048))
		{
			m_numVerticesPerDimension = bento::textureUtil::GetBestPowerOfTwo(m_numVerticesPerDimension);
			Invalidate();
		}
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
		std::vector<float> mappingData(m_numVertices * 4);

		std::srand(0);

		bool hexGrid = false;

		float cellSize = m_size / (float)m_numVerticesPerDimension;

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

				float xPos = (xRatio * m_size) - m_size * 0.5f;

				if (hexGrid)
				{
					if (i % 2 != 0)
					{
						xPos -= cellSize * 0.5f;
					}
				}

				positions[float3Index + 0] = xPos;
				positions[float3Index + 1] = 0.0f;
				positions[float3Index + 2] = (zRatio * m_size) - m_size * 0.5f;

				uvs[float2Index + 0] = xRatio;
				uvs[float2Index + 1] = zRatio;

				heightData[float4Index + 0] = 0.0f;
				heightData[float4Index + 1] = 0.0f;
				heightData[float4Index + 2] = 0.0f;
				heightData[float4Index + 3] = 0.0f;
			
				fluxData[float4Index + 0] = 0.0f;
				fluxData[float4Index + 1] = 0.0f;
				fluxData[float4Index + 2] = 0.0f;
				fluxData[float4Index + 3] = 0.0f;

				mappingData[float4Index + 0] = 0.0f;// 0.0f;// static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
				mappingData[float4Index + 1] = 0.0f;// static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
				mappingData[float4Index + 2] = 0.0f;// static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
				mappingData[float4Index + 3] = 0.0f;// static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);

				if (i < m_numVerticesPerDimension - 1 && j < m_numVerticesPerDimension - 1)
				{
					if (hexGrid)
					{
						if (i % 2 != 0)
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
						else
						{
							indices[indicesIndex] = vertexIndex;
							indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension;
							indices[indicesIndex + 2] = vertexIndex + m_numVerticesPerDimension + 1;
							indicesIndex += 3;

							indices[indicesIndex] = vertexIndex;
							indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension + 1;
							indices[indicesIndex + 2] = vertexIndex + 1;
							indicesIndex += 3;
						}
					}
					else
					{
						indices[indicesIndex] = vertexIndex;
						indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension;
						indices[indicesIndex + 2] = vertexIndex + m_numVerticesPerDimension + 1;
						indicesIndex += 3;

						indices[indicesIndex] = vertexIndex;
						indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension + 1;
						indices[indicesIndex + 2] = vertexIndex + 1;
						indicesIndex += 3;
					}
				}

				vertexIndex++;
			}
		}

		m_rockDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_rockDataA.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_rockDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_rockDataB.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_rockFluxDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_rockFluxDataA.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);
		m_rockFluxDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_rockFluxDataB.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);

		m_waterDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterDataA.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_waterDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterDataB.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_waterFluxDataA.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterFluxDataA.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);
		m_waterFluxDataB.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterFluxDataB.TexImage2D(GL_RGBA, GL_FLOAT, &fluxData[0]);

		m_mappingDataA.TexImage2D(GL_RGBA, GL_FLOAT, &mappingData[0]);
		m_mappingDataB.TexImage2D(GL_RGBA, GL_FLOAT, &mappingData[0]);
		
		BufferVertexData(0, &positions[0], positions.size());
		BufferVertexData(1, &uvs[0], uvs.size());
		BufferIndexData(0, &indices[0], indices.size());
	}

	void TerrainGeometry::OnInvalidate()
	{
		Geometry::OnInvalidate();
	}
}