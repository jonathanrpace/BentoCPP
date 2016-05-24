#pragma once

#include <string>

#include <gl/glew.h>
#include <bento/core/SharedObject.h>
#include <bento/core/Component.h>
#include <bento/core/AbstractValidatable.h>

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

		inline void Switch(bool _value) { m_switch = _value; }
		inline bool Switch() const { return m_switch; }

		inline GLuint VertexArrayA() { ValidateNow(); return m_vertexArrayA; }
		inline GLuint VertexArrayB() { ValidateNow();  return m_vertexArrayB; }

		inline GLuint TransformFeedbackObjA() { ValidateNow();  return m_transformFeedbackObjA; }
		inline GLuint TransformFeedbackObjB() { ValidateNow();  return m_transformFeedbackObjB; }

		inline int NumParticles() const { return m_numParticles; }

	protected:
		// From AbstractValidatable
		virtual void Validate() override;
		virtual void OnInvalidate() override;

	private:
		bool m_switch;
		int m_numParticles;
		GLuint m_vertexArrayA;
		GLuint m_vertexArrayB;
		GLuint m_positionBufferA;
		GLuint m_positionBufferB;
		GLuint m_propertiesBufferA;
		GLuint m_propertiesBufferB;		// TODO - This is a static VB. Why do I need two copies?
		GLuint m_transformFeedbackObjA;
		GLuint m_transformFeedbackObjB;
	};
}