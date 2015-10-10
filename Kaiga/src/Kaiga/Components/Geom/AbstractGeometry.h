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
		AbstractGeometry();
		~AbstractGeometry();
		
		// From IGeometry
		virtual void Bind() override;
		virtual void Draw() override;

	protected:
		virtual void OnInvalidate() override;

		void BufferVertexData(int _attributeIndex, float* _data, int _size);
		void BufferIndexData(int _index, int* _data, int _length);

		void SetVertexFormatf(int _attributeIndex, int _numComponents);
		void SetVertexFormati(int _attributeIndex, int _numComponents);

		GLuint m_vertexArrayName;
		int m_numIndices;
		int m_numVertices;

	private:
		static const int s_maxVertexBuffers = 8;
		static const int s_maxIndexBuffers = 8;
		GLuint* m_vertexBuffers;
		GLuint* m_indexBuffers;
	};
}