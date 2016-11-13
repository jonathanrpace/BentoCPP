#pragma once

#include <typeinfo>
#include <string>

// bento
#include <bento/core/Component.h>
#include <bento/core/SharedObject.h>
#include <bento/core/AbstractValidatable.h>

using namespace bento;

namespace godBox
{
	class MoltenPlateGeometry : 
		public AbstractValidatable,
		public Component,
		public SharedObject<MoltenPlateGeometry>
	{

	public:
		MoltenPlateGeometry(std::string _name = "MoltenPlateGeometry");

		GLuint PositionBuffer() 
		{ 
			ValidateNow();
			return m_positionBuffer;
		}

		GLuint IndexBuffer()
		{
			ValidateNow();
			return m_indexBuffer;
		}

		int NumVertices()
		{
			ValidateNow();
			return m_numVertices;
		}

		int NumIndices()
		{
			ValidateNow();
			return m_numIndices;
		}

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

		int m_numVertices;
		int m_numIndices;
		GLuint m_positionBuffer;
		GLuint m_indexBuffer;
	};
}