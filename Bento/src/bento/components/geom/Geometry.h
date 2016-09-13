#pragma once

#include <typeinfo>

#include <bento.h>

#include <bento/core/Component.h>
#include <bento/core/AbstractValidatable.h>

namespace bento
{
	class Geometry : 
		public Component, 
		public AbstractValidatable
	{
	public:
		Geometry(std::string _name = "Geometry", const std::type_info& _typeInfo = typeid(Geometry));
		~Geometry();
		
		void Bind();
		void Draw();

		int NumIndices()
		{ 
			Validate();
			return m_numIndices;
		}

	protected:
		// From AbstractValidatable
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