#include "ScreenQuadGeometry.h"

#include <glew.h>
#include <assert.h>

namespace Kaiga
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// PROTECTED
	//////////////////////////////////////////////////////////////////////////

	void ScreenQuadGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		// TODO - move vertex format to a base class function?
		glGenVertexArrays(1, &m_vertexArrayName);
		GL_CHECK(glBindVertexArray(m_vertexArrayName));
		SetVertexFormatf(0, 2);
		SetVertexFormatf(1, 2);
		GL_CHECK(glBindVertexArray(GL_NONE));

		m_numVertices = 4;
		
		float* positions = new float[m_numVertices*2];
		positions[0] = -1.0f; positions[1] =  1.0f; 
		positions[2] = -1.0f; positions[3] = -1.0f; 
		positions[4] =  1.0f; positions[5] = -1.0f;
		positions[6] =  1.0f; positions[7] =  1.0f;
		BufferVertexData(0, positions, m_numVertices * 2);
		delete positions;

		float* uvs = new float[m_numVertices*2];
		uvs[0] = 0.0f; uvs[1] = 1.0f; 
		uvs[2] = 0.0f; uvs[3] = 0.0f; 
		uvs[4] = 1.0f; uvs[5] = 0.0f;
		uvs[6] = 1.0f; uvs[7] = 1.0f;
		BufferVertexData(1, uvs, m_numVertices * 2);
		delete uvs;

		m_numIndices = 6;

		int* indices = new int[m_numIndices];
		indices[0] = 0; indices[1] = 1; indices[2] = 3;
		indices[3] = 3; indices[4] = 1; indices[5] = 2;
		BufferIndexData(0, indices, m_numIndices);
		delete indices;
	}
}