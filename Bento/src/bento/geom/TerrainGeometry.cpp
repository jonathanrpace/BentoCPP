#include "TerrainGeometry.h"

#include <random>

#include <bento.h>

namespace bento
{
	TerrainGeometry::TerrainGeometry()
		: m_size(1.0f)
		, m_numVerticesPerDimension(64)
		, m_texture0A()
		, m_texture0B()
		, m_texture1A()
		, m_texture1B()
		, m_texture2A()
		, m_texture2B()
		, m_texture0AIsWrite(true)
		, m_texture1AIsWrite(true)
		, m_texture2AIsWrite(true)
	{
	}

	TerrainGeometry::~TerrainGeometry()
	{
	}

	TextureSquare & TerrainGeometry::ReadTexture0()
	{
		return m_texture0AIsWrite ? m_texture0B : m_texture0A;
	}

	TextureSquare & TerrainGeometry::WriteTexture0()
	{
		return m_texture0AIsWrite ? m_texture0A : m_texture0B;
	}

	TextureSquare & TerrainGeometry::ReadTexture1()
	{
		return m_texture1AIsWrite ? m_texture0B : m_texture0A;
	}

	TextureSquare & TerrainGeometry::WriteTexture1()
	{
		return m_texture1AIsWrite ? m_texture1A : m_texture1B;
	}

	TextureSquare & TerrainGeometry::ReadTexture2()
	{
		return m_texture2AIsWrite ? m_texture2B : m_texture2A;
	}

	TextureSquare & TerrainGeometry::WriteTexture2()
	{
		return m_texture2AIsWrite ? m_texture2A : m_texture2B;
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

		std::vector<float> texture0Data(m_numVertices * 4);
		std::vector<float> texture1Data(m_numVertices * 4);
		std::vector<float> texture2Data(m_numVertices * 4);

		m_texture0A.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_texture0B.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_texture1A.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_texture1B.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_texture2A.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);
		m_texture2B.SetSize(m_numVerticesPerDimension, m_numVerticesPerDimension);

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

				texture0Data[float4Index + 0] = static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);	// solidHeight
				texture0Data[float4Index + 1] = 0.0f;	// moltenHeight
				texture0Data[float4Index + 2] = 0.0f;	// empty
				texture0Data[float4Index + 3] = 0.0f;	// empty

				texture1Data[float4Index + 0] = 0.0f;	// moltenVelocityU
				texture1Data[float4Index + 1] = 0.0f;	// moltenVelocityV
				texture1Data[float4Index + 2] = 0.0f;	// empty
				texture1Data[float4Index + 3] = 0.0f;	// empty

				texture2Data[float4Index + 0] = xRatio;	// textureU
				texture2Data[float4Index + 1] = zRatio;	// textureV
				texture2Data[float4Index + 2] = 0.0f;	// textureAngle
				texture2Data[float4Index + 3] = 0.0f;	// textureScale

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

		m_texture0A.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture0Data[0]);
		m_texture0B.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture0Data[0]);

		m_texture1A.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture1Data[0]);
		m_texture1B.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture1Data[0]);

		m_texture2A.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture2Data[0]);
		m_texture2B.TexImage2D(0, GL_RGBA, GL_FLOAT, &texture2Data[0]);

		BufferVertexData(0, &positions[0], m_numVertices * 3);
		BufferVertexData(1, &uvs[0], m_numVertices * 2);
		BufferIndexData(0, &indices[0], m_numIndices);
	}

	void TerrainGeometry::OnInvalidate()
	{
		GeometryBase::OnInvalidate();
	}

	const std::type_info & TerrainGeometry::typeInfo()
	{
		return typeid(TerrainGeometry);
	}
}