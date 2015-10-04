#pragma once

#include <vector>

#include <glew.h>
#include <kaiga.h>
#include <ramen.h>

namespace Kaiga
{
	class AbstractGeometry : 
		public IGeometry, 
		public Ramen::AbstractValidatable
	{
	public:
		virtual void Bind() override;
	protected:
		virtual void OnInvalidate() override;

		void BufferVertexData(int _attributeIndex, float * _data, int _size);
		void BufferIndexData(int _index, int* _data, int _length);

		void SetVertexFormatf(int _attributeIndex, int _numComponents);
		void SetVertexFormati(int _attributeIndex, int _numComponents);

		GLuint m_vertexArrayName;
		GLuint* m_vertexBuffers;
		int m_vertexBuffersSize;
		GLuint* m_indexBuffers;
		int m_indexBuffersSize;
		int m_numIndices;
		int m_numVertices;
	};
}