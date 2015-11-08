#include "PlaneGeometry.h"

#include <vector>
#include <glew.h>
#include <assert.h>

#include <bento.h>

namespace bento
{
	PlaneGeometry::PlaneGeometry(std::string _name)
		: Geometry(_name, typeid(Geometry))
		, m_width(1.0f)
		, m_height(1.0f)
		, m_numDivisionsX(0)
		, m_numDivisionsY(0)
	{

	}

	void PlaneGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		glGenVertexArrays(1, &m_vertexArrayName);
		GL_CHECK(glBindVertexArray(m_vertexArrayName));

		// Position;
		SetVertexFormatf(0, 3);

		// Normal
		SetVertexFormatf(1, 3);

		// UV
		SetVertexFormatf(2, 2);

		// Color
		SetVertexFormatf(3, 4);

		glBindVertexArray(0);

		int numVerticesX = 2 + m_numDivisionsX;
		int numVerticesY = 2 + m_numDivisionsY;
		m_numVertices = numVerticesX * numVerticesY;
		int numTriangles = (numVerticesX - 1) * (numVerticesY - 1) * 2;
		m_numIndices = numTriangles * 3;

		std::vector<float> positions(m_numVertices * 3);
		std::vector<float> normals(m_numVertices * 3);
		std::vector<float> uvs(m_numVertices * 2);
		std::vector<float> colors(m_numVertices * 4);
		std::vector<int> indices(m_numIndices);

		int indicesIndex = 0;
		int vertexIndex = 0;
		for (int i = 0; i < numVerticesY; i++)
		{
			float yRatio = (float)i / (numVerticesY - 1);
			for (int j = 0; j < numVerticesX; j++)
			{
				float xRatio = (float)j / (numVerticesX - 1);
				int float2Index = (i * numVerticesY + j) * 2;
				int float3Index = (i * numVerticesY + j) * 3;
				int float4Index = (i * numVerticesY + j) * 4;

				positions[float3Index] = (xRatio * m_width) - m_width * 0.5f;
				positions[float3Index + 1] = (yRatio * m_height) - m_height * 0.5f;
				positions[float3Index + 2] = 0.0f;

				normals[float3Index] = 0;
				normals[float3Index + 1] = 0;
				normals[float3Index + 2] = -1;

				uvs[float2Index] = xRatio;
				uvs[float2Index + 1] = 1 - yRatio;

				colors[float4Index] = xRatio;
				colors[float4Index + 1] = yRatio;
				colors[float4Index + 2] = 0;
				colors[float4Index + 3] = 0;

				if (i < numVerticesY - 1 && j < numVerticesX - 1)
				{
					indices[indicesIndex] = vertexIndex;
					indices[indicesIndex + 1] = vertexIndex + numVerticesX;
					indices[indicesIndex + 2] = vertexIndex + 1;
					indicesIndex += 3;

					indices[indicesIndex] = vertexIndex + 1;
					indices[indicesIndex + 1] = vertexIndex + numVerticesX;
					indices[indicesIndex + 2] = vertexIndex + numVerticesX + 1;
					indicesIndex += 3;
				}

				vertexIndex++;
			}
		}

		BufferVertexData(0, &positions[0], m_numVertices * 3);
		BufferVertexData(1, &normals[0], m_numVertices * 3);
		BufferVertexData(2, &uvs[0], m_numVertices * 2);
		BufferVertexData(3, &colors[0], m_numVertices * 4);
		BufferIndexData(0, &indices[0], m_numIndices);
	}

	float PlaneGeometry::GetWidth()
	{
		return m_width;
	}

	void PlaneGeometry::SetWidth(float _value)
	{
		m_width = _value;
		Invalidate();
	}

	float PlaneGeometry::GetHeight()
	{
		return m_height;
	}

	void PlaneGeometry::SetHeight(float _value)
	{
		m_height = _value;
		Invalidate();
	}

	int PlaneGeometry::GetNumDivisionsX()
	{
		return m_numDivisionsX;
	}

	void PlaneGeometry::SetNumDivisionsX(int _value)
	{
		m_numDivisionsX = _value;
		Invalidate();
	}

	int PlaneGeometry::GetNumDivisionsY()
	{
		return m_numDivisionsY;
	}

	void PlaneGeometry::SetNumDivisionsY(int _value)
	{
		m_numDivisionsY = _value;
		Invalidate();
	}
}