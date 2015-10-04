#include "PlaneGeometry.h"

#include <glew.h>
#include <assert.h>

Kaiga::PlaneGeometry::PlaneGeometry() :
	m_width(1.0f),
	m_height(1.0f),
	m_numDivisionsX(0),
	m_numDivisionsY(0)
{
	
}

Kaiga::PlaneGeometry::~PlaneGeometry()
{
	Invalidate();
}

void Kaiga::PlaneGeometry::Draw()
{
	GL_CHECK(glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr));
}

void Kaiga::PlaneGeometry::Validate()
{
	assert(glIsVertexArray(m_vertexArrayName) == false);

	glGenVertexArrays(1, &m_vertexArrayName);
	GL_CHECK(glBindVertexArray(m_vertexArrayName));

	m_vertexBuffersSize = 4;
	m_vertexBuffers = new GLuint[m_vertexBuffersSize];
	GL_CHECK(glGenBuffers(m_vertexBuffersSize, m_vertexBuffers));

	// Position;
	SetVertexFormatf(0, 3);
	
	// Normal
	SetVertexFormatf(1, 3);

	// UV
	SetVertexFormatf(2, 2);

	// Color
	SetVertexFormatf(3, 4);

	glBindVertexArray(0);

	m_indexBuffersSize = 1;
	m_indexBuffers = new GLuint[m_indexBuffersSize];
	glGenBuffers(m_indexBuffersSize, m_indexBuffers);

	int numVerticesX = 2 + m_numDivisionsX;
	int numVerticesY = 2 + m_numDivisionsY;
	m_numVertices = numVerticesX * numVerticesY;
	int numTriangles = (numVerticesX - 1) * (numVerticesY - 1) * 2;
	m_numIndices = numTriangles * 3;

	float* positions = new float[m_numVertices * 3];
	float* normals = new float[m_numVertices * 3];
	float* uvs = new float[m_numVertices * 2];
	float* colors = new float[m_numVertices * 4];
	int* indices = new int[m_numIndices];

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

	BufferVertexData(0, positions, m_numVertices * 3);
	BufferVertexData(1, normals, m_numVertices * 3);
	BufferVertexData(2, uvs, m_numVertices * 2);
	BufferVertexData(3, colors, m_numVertices * 4);
	BufferIndexData(0, indices, m_numIndices);

	delete positions;
	delete normals;
	delete uvs;
	delete colors;
	delete indices;
}

float Kaiga::PlaneGeometry::GetWidth()
{
	return m_width;
}

void Kaiga::PlaneGeometry::SetWidth(float _value)
{
	m_width = _value;
	Invalidate();
}

float Kaiga::PlaneGeometry::GetHeight()
{
	return m_height;
}

void Kaiga::PlaneGeometry::SetHeight(float _value)
{
	m_height = _value;
	Invalidate();
}

int Kaiga::PlaneGeometry::GetNumDivisionsX()
{
	return m_numDivisionsX;
}

void Kaiga::PlaneGeometry::SetNumDivisionsX(int _value)
{
	m_numDivisionsX = _value;
	Invalidate();
}

int Kaiga::PlaneGeometry::GetNumDivisionsY()
{
	return m_numDivisionsY;
}

void Kaiga::PlaneGeometry::SetNumDivisionsY(int _value)
{
	m_numDivisionsY = _value;
	Invalidate();
}