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
		, m_numVerticesPerDimension(256)
		, m_terrainMousePos()

		, m_heightData			(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_miscData			(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_normalData			(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_smudgeData			(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_waterFluxData		(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_uvOffsetData		(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)

		, m_fluidVelocityData	(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_densityData			(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_pressureData		(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
		, m_divergenceData		(m_numVerticesPerDimension>>0, GL_RGBA32F, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE)
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

		GL_CHECK(glBindVertexArray(0));

		m_numVertices = m_numVerticesPerDimension * m_numVerticesPerDimension;
		int numTriangles = (m_numVerticesPerDimension - 1) * (m_numVerticesPerDimension - 1) * 2;
		m_numIndices = numTriangles * 3;

		std::vector<float> positions(m_numVertices * 3);
		std::vector<float> uvs(m_numVertices * 2);
		std::vector<float> heightData(m_numVertices * 4);
		std::vector<float> randData(m_numVertices * 4);
		std::vector<int> indices(m_numIndices);

		std::srand(0);

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

				uvs[float2Index + 0] = xRatio;
				uvs[float2Index + 1] = zRatio;

				positions[float3Index + 0] = (xRatio * m_size) - m_size * 0.5f;
				positions[float3Index + 1] = 0.0f;
				positions[float3Index + 2] = (zRatio * m_size) - m_size * 0.5f;

				heightData[float4Index + 0] = 0.0f;
				heightData[float4Index + 1] = 0.0f;
				heightData[float4Index + 2] = 0.0f;
				heightData[float4Index + 3] = 0.0f;

				if (i < m_numVerticesPerDimension - 1 && j < m_numVerticesPerDimension - 1)
				{
					// ALternate triangulation
					if ( j % 2 == 0 )
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
					else
					{
						indices[indicesIndex] = vertexIndex;
						indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension + 1;
						indices[indicesIndex + 2] = vertexIndex + 1;
						indicesIndex += 3;

						indices[indicesIndex] = vertexIndex;
						indices[indicesIndex + 1] = vertexIndex + m_numVerticesPerDimension;
						indices[indicesIndex + 2] = vertexIndex + m_numVerticesPerDimension + 1;
						indicesIndex += 3;
					}
				}

				vertexIndex++;
			}
		}

		m_heightData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_heightData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_miscData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_normalData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_smudgeData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_waterFluxData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_uvOffsetData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		m_densityData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_densityData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_pressureData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_pressureData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_divergenceData.TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_fluidVelocityData.GetRead().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);
		m_fluidVelocityData.GetWrite().TexImage2D(GL_RGBA, GL_FLOAT, &heightData[0]);

		BufferVertexData(0, &positions[0], (int)positions.size());
		BufferVertexData(1, &uvs[0], (int)uvs.size());
		BufferIndexData(0, &indices[0], (int)indices.size());
	}

	void TerrainGeometry::OnInvalidate()
	{
		Geometry::OnInvalidate();
	}
}