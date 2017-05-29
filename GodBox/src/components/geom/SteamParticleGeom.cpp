#include "SteamParticleGeom.h"

#include <bento/util/Rand.h>
#include <math.h>

namespace godBox
{
	SteamParticleGeom::SteamParticleGeom(std::string _name)
		: Component(_name, typeid(SteamParticleGeom)),
		m_vertexArrayA(-1),
		m_vertexArrayB(-1),
		m_numParticles(5000)
	{

	}

	SteamParticleGeom::~SteamParticleGeom()
	{
		Invalidate();
	}

	void SteamParticleGeom::Validate()
	{
		assert(glIsVertexArray(m_vertexArrayA) == false);
		
		// Generate all the shit
		std::vector<float> zeroValues(m_numParticles * 4);
		std::vector<float> properties(m_numParticles * 4);

		float numParticlesPerDimension = (float)std::sqrt((float)m_numParticles);

		for (int i = 0; i < m_numParticles; i++)
		{
			int float3Index = i * 3;
			int float4Index = i * 4;
			float fi = (float)i;
			float xRatio = std::fmod( fi / numParticlesPerDimension, 1.0f );
			float yRatio = std::floor(fi / numParticlesPerDimension) / numParticlesPerDimension;

			zeroValues[float4Index + 0] = 0.0f;
			zeroValues[float4Index + 1] = 0.0f;
			zeroValues[float4Index + 2] = 0.0f;
			zeroValues[float4Index + 3] = 0.0f;
			
			properties[float4Index + 0] = xRatio;	// HomeX
			properties[float4Index + 1] = yRatio;	// HomeY
			properties[float4Index + 2] = 0.0f;
			properties[float4Index + 3] = 0.0f;
		}

		// Transfer the data to the buffers, and bind them together, associating some with transform feedback
		{
			//////////////////////////////////////////////////////////////////////////////////////////////
			// A
			//////////////////////////////////////////////////////////////////////////////////////////////

			GL_CHECK(glGenTransformFeedbacks(1, &m_transformFeedbackObjA));
			GL_CHECK(glGenVertexArrays(1, &m_vertexArrayA));
			GL_CHECK(glGenBuffers(1, &m_buffer0A));
			GL_CHECK(glGenBuffers(1, &m_buffer1A));
			GL_CHECK(glGenBuffers(1, &m_buffer2A));
			GL_CHECK(glGenBuffers(1, &m_propertiesBufferA));

			GL_CHECK(glBindVertexArray(m_vertexArrayA));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedbackObjA));

			// Buffer 0 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer0A));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_buffer0A));
			
			// Buffer 1 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer1A));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_buffer1A));

			// Buffer 2 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer2A));
			GL_CHECK(glEnableVertexAttribArray(2));
			GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, m_buffer2A));

			// Properties A (No transform feedback)
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_propertiesBufferA));
			GL_CHECK(glEnableVertexAttribArray(3));
			GL_CHECK(glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, properties.size() * sizeof(float), &properties[0], GL_DYNAMIC_COPY));

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, GL_NONE));

			//////////////////////////////////////////////////////////////////////////////////////////////
			// B
			//////////////////////////////////////////////////////////////////////////////////////////////
			
			GL_CHECK(glGenTransformFeedbacks(1, &m_transformFeedbackObjB));
			GL_CHECK(glGenVertexArrays(1, &m_vertexArrayB));
			GL_CHECK(glGenBuffers(1, &m_buffer0B));
			GL_CHECK(glGenBuffers(1, &m_buffer1B));
			GL_CHECK(glGenBuffers(1, &m_buffer2B));
			GL_CHECK(glGenBuffers(1, &m_propertiesBufferB));

			GL_CHECK(glBindVertexArray(m_vertexArrayB));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedbackObjB));

			// Buffer 0 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer0B));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_buffer0B));
			
			// Buffer 1 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer1B));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_buffer1B));

			// Buffer 2 A
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_buffer2B));
			GL_CHECK(glEnableVertexAttribArray(2));
			GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, zeroValues.size() * sizeof(float), &zeroValues[0], GL_DYNAMIC_COPY));
			GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, m_buffer2B));
			
			// Properties B (No transform feedback)
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_propertiesBufferB));
			GL_CHECK(glEnableVertexAttribArray(3));
			GL_CHECK(glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(float) * 4, nullptr));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, properties.size() * sizeof(float), &properties[0], GL_DYNAMIC_COPY));
			
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, GL_NONE));
			GL_CHECK(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, GL_NONE));
		}
	}

	void SteamParticleGeom::OnInvalidate()
	{
		if (glIsVertexArray(m_vertexArrayA))
		{
			glDeleteVertexArrays(1, &m_vertexArrayA);
			glDeleteVertexArrays(1, &m_vertexArrayB);

			glDeleteBuffers(1, &m_buffer0A);
			glDeleteBuffers(1, &m_buffer0B);
			glDeleteBuffers(1, &m_buffer1A);
			glDeleteBuffers(1, &m_buffer1B);
			glDeleteBuffers(1, &m_buffer2A);
			glDeleteBuffers(1, &m_buffer2B);
			glDeleteBuffers(1, &m_propertiesBufferA);
			glDeleteBuffers(1, &m_propertiesBufferB);

			glDeleteTransformFeedbacks(1, &m_transformFeedbackObjA);
			glDeleteTransformFeedbacks(1, &m_transformFeedbackObjB);
		}
	}
}