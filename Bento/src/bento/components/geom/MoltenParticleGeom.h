#pragma once

#include <string>

#include <gl/glew.h>
#include <bento/core/SharedObject.h>
#include <bento/core/Component.h>
#include <bento/core/AbstractValidatable.h>

#include <bento/components/geom/MoltenPlateGeometry.h>

namespace bento
{
	class MoltenParticleGeom :
		public Component,
		public AbstractValidatable,
		public SharedObject<MoltenParticleGeom>
	{
	public:
		MoltenParticleGeom(std::string _name = "MoltenParticleGeom");
		~MoltenParticleGeom();

		void Draw();

		inline void Switch() 
		{ 
			m_switch = !m_switch;
		}

		inline GLuint ParticleVertexArrayRead()
		{
			ValidateNow();
			return m_switch ? m_particleVertexArrayA : m_particleVertexArrayB;
		}

		inline GLuint ParticleVertexArrayWrite()
		{
			ValidateNow();
			return m_switch ? m_particleVertexArrayB : m_particleVertexArrayA;
		}

		inline GLuint TransformFeedbackObjRead() 
		{ 
			ValidateNow();  
			return m_switch ? m_transformFeedbackObjA : m_transformFeedbackObjB; 
		}

		inline GLuint TransformFeedbackObjWrite()
		{ 
			ValidateNow(); 
			return m_switch ? m_transformFeedbackObjB : m_transformFeedbackObjA;
		}

		inline int NumParticles() const
		{ 
			return m_numParticles;
		}

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		bool m_switch;
		int m_numParticles;
		GLuint m_particleVertexArrayA;
		GLuint m_particleVertexArrayB;
		GLuint m_positionBufferA;
		GLuint m_positionBufferB;
		GLuint m_propertiesBufferA;
		GLuint m_propertiesBufferB;		// TODO - This is a static VB. Why do I need two copies?
		GLuint m_transformFeedbackObjA;
		GLuint m_transformFeedbackObjB;
		GLuint m_drawVertexArrayA;
		GLuint m_drawVertexArrayB;

		MoltenPlateGeometry m_moltenPlateGeom;
	};
}