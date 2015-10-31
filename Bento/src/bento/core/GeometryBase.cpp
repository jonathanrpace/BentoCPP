#include "GeometryBase.h"

bento::GeometryBase::GeometryBase()
{
	m_vertexBuffers = new GLuint[s_maxVertexBuffers];
	glGenBuffers(s_maxVertexBuffers, m_vertexBuffers);

	m_indexBuffers = new GLuint[s_maxIndexBuffers];
	glGenBuffers(s_maxIndexBuffers, m_indexBuffers);
}

bento::GeometryBase::~GeometryBase()
{
	Invalidate();

	delete m_vertexBuffers;
	delete m_indexBuffers;
}

void bento::GeometryBase::Bind()
{
	ValidateNow();

	GL_CHECK(glBindVertexArray(m_vertexArrayName));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[0]));
}

void bento::GeometryBase::Draw()
{
	GL_CHECK(glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr));
}

const std::type_info & bento::GeometryBase::typeInfo()
{
	return typeid(GeometryBase);
}

void bento::GeometryBase::OnInvalidate()
{
	if (glIsVertexArray(m_vertexArrayName))
	{
		GL_CHECK(glDeleteVertexArrays(1, &m_vertexArrayName));
	}
}

void bento::GeometryBase::BufferVertexData(int _attributeIndex, float* _data, int _size)
{
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[_attributeIndex]));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, _size * sizeof(float), _data, GL_STATIC_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
}

void bento::GeometryBase::BufferIndexData(int _index, int* _data, int _size)
{
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffers[_index]));
	GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _size * sizeof(int), _data, GL_STATIC_DRAW));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
}

void bento::GeometryBase::SetVertexFormatf(int _attributeIndex, int _numComponents)
{
	GL_CHECK(glEnableVertexAttribArray(_attributeIndex));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[_attributeIndex]));
	GL_CHECK(glVertexAttribPointer(_attributeIndex, _numComponents, GL_FLOAT, false, sizeof(float) * _numComponents, nullptr));
}

void bento::GeometryBase::SetVertexFormati(int _attributeIndex, int _numComponents)
{
	GL_CHECK(glEnableVertexAttribArray(_attributeIndex));
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[_attributeIndex]));
	GL_CHECK(glVertexAttribPointer(_attributeIndex, _numComponents, GL_INT, false, sizeof(int) * _numComponents, nullptr));
}

