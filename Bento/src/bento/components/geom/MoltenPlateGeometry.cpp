#include "MoltenPlateGeometry.h"

#include <bento.h>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <bento/util/Rand.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////

	MoltenPlateGeometry::MoltenPlateGeometry(std::string _name)
		: Component(_name, typeid(MoltenPlateGeometry))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// PROTECTED
	//////////////////////////////////////////////////////////////////////////

	void MoltenPlateGeometry::Validate()
	{
		assert(glIsBuffer(m_positionBuffer) == false);
		assert(glIsBuffer(m_indexBuffer) == false);

		GL_CHECK(glGenBuffers(1, &m_positionBuffer));
		GL_CHECK(glGenBuffers(1, &m_indexBuffer));

		const int numPoints = 7;
		const float outerRadius = 1.0f;
		const float innerRadius = 0.8f;

		int numTriangles = numPoints * 2 + numPoints;

		m_numVertices = numPoints * 2 + 1;
		m_numIndices = numTriangles * 3;


		float* positions = new float[m_numVertices*3];
		int* indices = new int[m_numIndices];

		std::srand(0);

		int positionIndex = 0;
		int indicesIndex = 0;
		int vertexIndex = 0;

		for ( int i = 0; i < numPoints; i++ )
		{
			float angleRatio = (float)i / numPoints;

			float cos = std::cosf(angleRatio * (float)M_PI);
			float sin = std::sinf(angleRatio * (float)M_PI);

			float radiusScalar = Rand(0.8f, 1.0f);

			float innerX = cos * innerRadius * radiusScalar;
			float innerY = sin * innerRadius * radiusScalar;
			float outerX = cos * outerRadius * radiusScalar;
			float outerY = sin * outerRadius * radiusScalar;

			if ( i > 0 )
			{
				indices[indicesIndex++] = vertexIndex;
				indices[indicesIndex++] = vertexIndex+1;
				indices[indicesIndex++] = vertexIndex-2;

				indices[indicesIndex++] = vertexIndex+1;
				indices[indicesIndex++] = vertexIndex-3;
				indices[indicesIndex++] = vertexIndex-2;
			}

			int index = i * 6;
			positions[positionIndex++] = innerX;
			positions[positionIndex++] = innerY;
			positions[positionIndex++] = 1.0f;
			vertexIndex++;

			positions[positionIndex++] = outerX;
			positions[positionIndex++] = outerY;
			positions[positionIndex++] = 0.0f;
			vertexIndex++;
		}

		// Center
		positions[positionIndex++] = 0.0f;
		positions[positionIndex++] = 0.0f;
		positions[positionIndex++] = 1.0f;

		for ( int i = 0; i < numPoints; i++ )
		{
			indices[indicesIndex++] = vertexIndex;
			indices[indicesIndex++] = (i+1) * 2;
			indices[indicesIndex++] = i * 2;
		}

		GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer) );
		GL_CHECK( glBufferData(GL_ARRAY_BUFFER, m_numVertices * 3 * sizeof(float), positions, GL_STATIC_DRAW) );
		GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, GL_NONE) );

		GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer) );
		GL_CHECK( glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndices * sizeof(int), indices, GL_STATIC_DRAW) );
		GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE) );
		
		delete positions;
		delete indices;
	}

	void MoltenPlateGeometry::OnInvalidate()
	{
		assert(glIsBuffer(m_positionBuffer));
		assert(glIsBuffer(m_indexBuffer));

		GL_CHECK(glDeleteBuffers(1, &m_positionBuffer));
		GL_CHECK(glDeleteBuffers(1, &m_indexBuffer));
	}
}