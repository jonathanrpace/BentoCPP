#include "TerrainGeometry.h"

#include <random>

#include <bento.h>
#include <imgui.h>

#include <bento/util/Rand.h>
#include <bento/util/TextureUtil.h>

namespace godBox
{
	TerrainGeometry::TerrainGeometry(std::string _name)
		: Geometry(_name, typeid(TerrainGeometry))
		, m_size(1.0f)
		, m_numVerticesPerDimension(512)
		, m_terrainMousePos()

		, m_heightData		(m_numVerticesPerDimension,	GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_velocityData	(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_miscData		(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_normalData		(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_smudgeData		(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
		, m_waterFluxData	(m_numVerticesPerDimension, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP, GL_CLAMP)
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

		GL_CHECK(glGenVertexArrays(1, &m_vertexArrayName));
		GL_CHECK(glBindVertexArray(m_vertexArrayName));

		SetVertexFormatf(0, 3);	// Position;
		SetVertexFormatf(1, 2);	// UV;
		SetVertexFormatf(2, 4);	// Rand;

		GL_CHECK(glBindVertexArray(0));

		m_numVertices = m_numVerticesPerDimension * m_numVerticesPerDimension;
		int numTriangles = (m_numVerticesPerDimension - 1) * (m_numVerticesPerDimension - 1) * 2;
		m_numIndices = numTriangles * 3;

		std::vector<float> positions(m_numVertices * 3);
		std::vector<float> uvs(m_numVertices * 2);
		std::vector<float> heightData(m_numVertices * 4);
		std::vector<float> randData(m_numVertices * 4);
		std::vector<int> indices(m_numIndices);

		//std::vector<float> fluxData(m_numVertices * 4);
		//std::vector<float> mappingData(m_numVertices * 4);

		std::srand(0);

		bool hexGrid = true;

		float cellSize = m_size / (float)m_numVerticesPerDimension;
		float uvPerCell = 1.0f / (float)m_numVerticesPerDimension;

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
					uvs[float2Index + 1] = zRatio;

					if (i % 2 != 0)
					{
						xPos -= cellSize * 0.5f;
						uvs[float2Index + 0] = xRatio - uvPerCell * 0.5f;
					}
					else
					{
						uvs[float2Index + 0] = xRatio;
					}
				}
				else
				{
					uvs[float2Index + 0] = xRatio;
					uvs[float2Index + 1] = zRatio;
				}

				positions[float3Index + 0] = xPos;
				positions[float3Index + 1] = 0.0f;
				positions[float3Index + 2] = (zRatio * m_size) - m_size * 0.5f;

				heightData[float4Index + 0] = 0.0f;//(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX)) * 0.1f;
				heightData[float4Index + 1] = 0.0f;//(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX)) * 0.1f;
				heightData[float4Index + 2] = 0.0f;//(static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX)) * 0.1f;
				heightData[float4Index + 3] = 0.0f;// (static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX)) * 0.001f;

				randData[float4Index + 0] = Rand();
				randData[float4Index + 1] = Rand();
				randData[float4Index + 2] =	Rand();
				randData[float4Index + 3] =	Rand();

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

		m_heightData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_heightData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_heightData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_heightData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_velocityData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_velocityData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_velocityData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_velocityData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_miscData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_miscData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_miscData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_miscData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_normalData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_normalData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_normalData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_normalData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_smudgeData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_smudgeData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_smudgeData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_smudgeData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_waterFluxData.GetRead().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterFluxData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_waterFluxData.GetWrite().SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_waterFluxData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		BufferVertexData(0, &positions[0], (int)positions.size());
		BufferVertexData(1, &uvs[0], (int)uvs.size());
		BufferVertexData(2, &randData[0], (int)randData.size());
		BufferIndexData(0, &indices[0], (int)indices.size());
	}

	void TerrainGeometry::OnInvalidate()
	{
		Geometry::OnInvalidate();
	}
}