#include "FoamParticleGeom.h"

#include <bento/util/Rand.h>
#include <math.h>

namespace godBox
{
	FoamParticleGeom::FoamParticleGeom(std::string _name)
		: Component(_name, typeid(FoamParticleGeom)),
		m_vertexArrayA(-1),
		m_vertexArrayB(-1),
		m_numParticles(100000)
	{

	}

	FoamParticleGeom::~FoamParticleGeom()
	{
		Invalidate();
	}

	void FoamParticleGeom::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayA) == false);
		
		// Generate all the shit
		//std::vector<float> positions(m_numParticles * 4);
		std::vector<float> velocities(m_numParticles * 4);
		std::vector<float> properties(m_numParticles * 4);

		float numParticlesPerDimension = (float)std::sqrt((float)m_numParticles);



		for (int i = 0; i < m_numParticles; i++)
		{
			int float3Index = i * 3;
			int float4Index = i * 4;

			//positions[float4Index + 0] = 0.0f;
			//positions[float4Index + 1] = 0.0f;
			//positions[float4Index + 2] = 0.0f;
			//positions[float4Index + 3] = 1.0f;

			velocities[float4Index + 0] = 0.0f;
			velocities[float4Index + 1] = 0.0f;
			velocities[float4Index + 2] = 0.0f;
			velocities[float4Index + 3] = -0.1f;	// Life

			float fi = (float)i;
			float xRatio = std::fmod( fi / numParticlesPerDimension, 1.0f );
			float yRatio = std::floor(fi / numParticlesPerDimension) / numParticlesPerDimension;

			properties[float4Index + 0] = xRatio;			// HomeX
			properties[float4Index + 1] = yRatio;			// HomeY
			properties[float4Index + 2] = Rand();		// Max life
			properties[float4Index + 3] = Rand();			// Handy randy
		}

		// Transfer the data to the buffers, and bind them together, associating some with transform feedback
		{
			//////////////////////////////////////////////////////////////////////////////////////////////
			// A
			//////////////////////////////////////////////////////////////////////////////////////////////

			GL_CHECK(glGenTransformFeedbacks(1, &m_transformFeedbackObjA));
			GL_CHECK(glGenVertexArrays(1, &m_vertexArrayA));
			GL_CHECK(glGenBuffers(1, &m_positionBufferA));
			GL_CHECK(glGenBuffers(1, &m_velocityBufferA));
			GL_CHECK(glGenBuffers(1, &m_propertiesBufferA));

			GL_CHECK(glBindVertexArray(m_vertexArrayA));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedbackObjA));

			// Position A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferA));											// Start doing stuff with position buffer A
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));	// Mark up this array as being the zero index attribute.
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(float), &velocities[0], GL_DYNAMIC_COPY));			// Transfer the data across
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_positionBufferA));						// Position will be fedback to 0 index buffer.
			
			// Velocity A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_velocityBufferA));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(float), &velocities[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_velocityBufferA));
			
			// Properties A (No transform feedback)
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_propertiesBufferA));
			GL_CHECK(glEnableVertexAttribArray(2));
			GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, properties.size() * sizeof(float), &properties[0], GL_DYNAMIC_COPY));

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, GL_NONE));

			//////////////////////////////////////////////////////////////////////////////////////////////
			// B
			//////////////////////////////////////////////////////////////////////////////////////////////
			
			GL_CHECK(glGenTransformFeedbacks(1, &m_transformFeedbackObjB));
			GL_CHECK(glGenVertexArrays(1, &m_vertexArrayB));
			GL_CHECK(glGenBuffers(1, &m_positionBufferB));
			GL_CHECK(glGenBuffers(1, &m_velocityBufferB));
			GL_CHECK(glGenBuffers(1, &m_propertiesBufferB));

			GL_CHECK(glBindVertexArray(m_vertexArrayB));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedbackObjB));

			// Position B
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_positionBufferB));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(float), &velocities[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_positionBufferB));
			
			// Velocity B
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_velocityBufferB));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, velocities.size() * sizeof(float), &velocities[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_velocityBufferB));
			
			// Properties B (No transform feedback)
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_propertiesBufferB));
			GL_CHECK(glEnableVertexAttribArray(2));
			GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, properties.size() * sizeof(float), &properties[0], GL_DYNAMIC_COPY));
			
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, GL_NONE));
		}
	}

	void FoamParticleGeom::OnInvalidate()
	{
		if (glIsVertexArray(m_vertexArrayA))
		{
			glDeleteVertexArrays(1, &m_vertexArrayA);
			glDeleteVertexArrays(1, &m_vertexArrayB);

			glDeleteBuffers(1, &m_positionBufferA);
			glDeleteBuffers(1, &m_positionBufferB);
			glDeleteBuffers(1, &m_velocityBufferA);
			glDeleteBuffers(1, &m_velocityBufferB);
			glDeleteBuffers(1, &m_propertiesBufferA);
			glDeleteBuffers(1, &m_propertiesBufferB);

			glDeleteTransformFeedbacks(1, &m_transformFeedbackObjA);
			glDeleteTransformFeedbacks(1, &m_transformFeedbackObjB);
		}
	}
}