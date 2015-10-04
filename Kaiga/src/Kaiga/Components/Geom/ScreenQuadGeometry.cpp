#include "ScreenQuadGeometry.h"

#include <glew.h>
#include <assert.h>

namespace Kaiga
{
	//////////////////////////////////////////////////////////////////////////
	// PUBLIC
	//////////////////////////////////////////////////////////////////////////

	void ScreenQuadGeometry::Draw()
	{
		GL_CHECK(glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr));
	}

	//////////////////////////////////////////////////////////////////////////
	// PROTECTED
	//////////////////////////////////////////////////////////////////////////

	void ScreenQuadGeometry::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayName) == false);

		glGenVertexArrays(1, &m_vertexArrayName);
		GL_CHECK(glBindVertexArray(m_vertexArrayName));

		m_vertexBuffersSize = 2;
		m_vertexBuffers = new GLuint[m_vertexBuffersSize];
		glGenBuffers(m_vertexBuffersSize, m_vertexBuffers);

		SetVertexFormatf(0, 2);
		SetVertexFormatf(1, 2);

		GL_CHECK(glBindVertexArray(GL_NONE));

		m_indexBuffersSize = 1;
		m_indexBuffers = new GLuint[1];
		glGenBuffers(m_indexBuffersSize, m_indexBuffers);

		m_numVertices = 4;
		m_numIndices = 6;

		float positions[] = { -1, 1, -1, -1, 1, -1, 1, 1 };
		float uvs[] = { 0, 1, 0, 0, 1, 0, 1, 1 };
		int indices[] = { 0, 1, 3, 3, 1, 2 };

		BufferVertexData(0, positions, m_numVertices * 2);
		BufferVertexData(1, uvs, m_numVertices * 2);
		BufferIndexData(0, indices, m_numIndices);
	}
}