#include "TerrainSidesGeometry.h"

#include <random>

#include <bento.h>
#include <imgui.h>

#include <bento/util/Rand.h>
#include <bento/util/TextureUtil.h>

namespace godBox
{
	TerrainSidesGeometry::TerrainSidesGeometry(std::string _name)
		: Geometry(_name, typeid(TerrainSidesGeometry))
		, m_size(1.0f)
		, m_height(0.1f)
		, m_numVerticesPerDimension(512)
	{
		
	}

	TerrainSidesGeometry::~TerrainSidesGeometry()
	{
	}

	void TerrainSidesGeometry::AddUIElements()
	{
		
	}

	void TerrainSidesGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		GL_CHECK(glGenVertexArrays(1, &m_vertexArrayName));
		GL_CHECK(glBindVertexArray(m_vertexArrayName));

		SetVertexFormatf(0, 3);	// Position;
		SetVertexFormatf(1, 3);	// UV;

		GL_CHECK(glBindVertexArray(0));

		m_numVertices = m_numVerticesPerDimension * 2 * 4;
		const int numTriangles = (m_numVerticesPerDimension - 1) * 2 * 4;
		m_numIndices = numTriangles * 3;

		std::vector<float> positions(m_numVertices * 3);
		std::vector<float> uvs(m_numVertices * 3);
		std::vector<int> indices(m_numIndices);

		std::srand(0);

		float cellSize = m_size / (float)m_numVerticesPerDimension;
		float uvPerCell = 1.0f / (float)m_numVerticesPerDimension;
		float halfSize = m_size * 0.5f;

		int indicesIndex = 0;
		int vertexIndex = 0;
		int positionIndex = 0;
		int uvIndex = 0;

		// Left edge
		for (int i = 0; i < m_numVerticesPerDimension; i++)
		{
			float ratio = (float)i / (m_numVerticesPerDimension-1);

			positions[positionIndex++] = -halfSize;
			positions[positionIndex++] = 0.0f;
			positions[positionIndex++] = ( ratio * m_size ) - halfSize;

			uvs[uvIndex++] = 0.0f;
			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 1.0f;

			positions[positionIndex++] = -halfSize;
			positions[positionIndex++] = -m_height;
			positions[positionIndex++] = ( ratio * m_size ) - halfSize;
			
			uvs[uvIndex++] = 0.0f;
			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 0.0f;

			if ( i > 0 )
			{
				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-2;
				indices[indicesIndex++] = vertexIndex-1;

				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-1;
				indices[indicesIndex++] = vertexIndex+1;
			}
			
			vertexIndex += 2;
		}

		// Right edge
		for (int i = 0; i < m_numVerticesPerDimension; i++)
		{
			float ratio = (float)i / (m_numVerticesPerDimension-1);

			positions[positionIndex++] = halfSize;
			positions[positionIndex++] = 0.0f;
			positions[positionIndex++] = ( ratio * m_size ) - halfSize;

			uvs[uvIndex++] = 1.0f;
			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 1.0f;

			positions[positionIndex++] = halfSize;
			positions[positionIndex++] = -m_height;
			positions[positionIndex++] = ( ratio * m_size ) - halfSize;
			
			uvs[uvIndex++] = 1.0f;
			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 0.0f;

			if ( i > 0 )
			{
				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-1;
				indices[indicesIndex++] = vertexIndex-2;

				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex+1;
				indices[indicesIndex++] = vertexIndex-1;
			}
			
			vertexIndex += 2;
		}

		// Bottom edge
		for (int i = 0; i < m_numVerticesPerDimension; i++)
		{
			float ratio = (float)i / (m_numVerticesPerDimension-1);

			positions[positionIndex++] = (ratio * m_size) - halfSize;
			positions[positionIndex++] = 0.0f;
			positions[positionIndex++] = halfSize;

			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 1.0f;
			uvs[uvIndex++] = 1.0f;

			positions[positionIndex++] = (ratio * m_size) - halfSize;
			positions[positionIndex++] = -m_height;
			positions[positionIndex++] = halfSize;

			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 1.0f;
			uvs[uvIndex++] = 0.0f;
			
			if ( i > 0 )
			{
				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-2;
				indices[indicesIndex++] = vertexIndex-1;

				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-1;
				indices[indicesIndex++] = vertexIndex+1;
			}
			
			vertexIndex += 2;
		}

		// Top edge
		for (int i = 0; i < m_numVerticesPerDimension; i++)
		{
			float ratio = (float)i / (m_numVerticesPerDimension-1);

			positions[positionIndex++] = (ratio * m_size) - halfSize;
			positions[positionIndex++] = 0.0f;
			positions[positionIndex++] = -halfSize;

			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 0.0f;
			uvs[uvIndex++] = 1.0f;

			positions[positionIndex++] = (ratio * m_size) - halfSize;
			positions[positionIndex++] = -m_height;
			positions[positionIndex++] = -halfSize;
			
			uvs[uvIndex++] = ratio;
			uvs[uvIndex++] = 0.0f;
			uvs[uvIndex++] = 0.0f;

			if ( i > 0 )
			{
				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex-1;
				indices[indicesIndex++] = vertexIndex-2;

				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex+1;
				indices[indicesIndex++] = vertexIndex-1;
			}
			
			vertexIndex += 2;
		}

		BufferVertexData(0, &positions[0], (int)positions.size());
		BufferVertexData(1, &uvs[0], (int)uvs.size());
		BufferIndexData(0, &indices[0], (int)indices.size());
	}

	void TerrainSidesGeometry::OnInvalidate()
	{
		Geometry::OnInvalidate();
	}
}