#include "SkyboxGeometry.h"

#include <bento.h>

namespace bento
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////

	SkyboxGeometry::SkyboxGeometry(std::string _name)
		:Geometry(_name, typeid(SkyboxGeometry))
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// PROTECTED
	//////////////////////////////////////////////////////////////////////////

	void SkyboxGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		glGenVertexArrays(1, &m_vertexArrayName);
		GL_CHECK(glBindVertexArray(m_vertexArrayName));
		SetVertexFormatf(0, 3);	// position
		GL_CHECK(glBindVertexArray(GL_NONE));

		m_numVertices = 8;
		
		float* positions = new float[m_numVertices*3];
		positions[ 0] = -1.0f; positions[ 1] =  1.0f; positions[ 2] =  1.0f;
		positions[ 3] =  1.0f; positions[ 4] =  1.0f; positions[ 5] =  1.0f;
		positions[ 6] = -1.0f; positions[ 7] = -1.0f; positions[ 8] =  1.0f;
		positions[ 9] =  1.0f; positions[10] = -1.0f; positions[11] =  1.0f;
		positions[12] = -1.0f; positions[13] =  1.0f; positions[14] = -1.0f;
		positions[15] =  1.0f; positions[16] =  1.0f; positions[17] = -1.0f;
		positions[18] = -1.0f; positions[19] = -1.0f; positions[20] = -1.0f;
		positions[21] =  1.0f; positions[22] = -1.0f; positions[23] = -1.0f;
		BufferVertexData(0, positions, m_numVertices * 3);
		delete positions;

		const int numFaces = 12;
		m_numIndices = numFaces * 3;

		int* indices = new int[m_numIndices];

		// X+
		indices[ 0] = 1; indices[ 1] = 3; indices[ 2] = 5;
		indices[ 3] = 5; indices[ 4] = 3; indices[ 5] = 7;

		// X-
		indices[ 6] = 4; indices[ 7] = 6; indices[ 8] = 0;
		indices[ 9] = 0; indices[10] = 6; indices[11] = 2;

		// Y+
		indices[12] = 4; indices[13] = 0; indices[14] = 5;
		indices[15] = 5; indices[16] = 0; indices[17] = 1;

		// Y-
		indices[18] = 2; indices[19] = 6; indices[20] = 3;
		indices[21] = 3; indices[22] = 6; indices[23] = 7;

		// Z+
		indices[24] = 0; indices[25] = 2; indices[26] = 1;
		indices[27] = 1; indices[28] = 2; indices[29] = 3;

		// Z-
		indices[30] = 5; indices[31] = 7; indices[32] = 4;
		indices[33] = 4; indices[34] = 7; indices[35] = 6;

		BufferIndexData(0, indices, m_numIndices);
		delete indices;
	}
}