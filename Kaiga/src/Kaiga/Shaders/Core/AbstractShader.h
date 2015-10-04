#pragma once

#include <kaiga.h>

namespace Kaiga
{
	template<typename T, typename U>
	class AbstractShader
	{
	public:
		AbstractShader() :
			m_pipelineName(0),
			m_vertexShader(),
			m_fragmentShader()
		{
			GL_CHECK(glGenProgramPipelines(1, &m_pipelineName));
			GL_CHECK(glUseProgramStages(m_pipelineName, GL_VERTEX_SHADER_BIT, m_vertexShader.GetProgramName()));
			GL_CHECK(glUseProgramStages(m_pipelineName, GL_FRAGMENT_SHADER_BIT, m_fragmentShader.GetProgramName()));
			GL_CHECK(glBindProgramPipeline(GL_NONE));
		}
		~AbstractShader()
		{
			GL_CHECK(glDeleteProgramPipelines(1, &m_pipelineName));
		}

		void BindPerPass()
		{
			GL_CHECK(glBindProgramPipeline(m_pipelineName));
			GL_CHECK(glActiveShaderProgram(m_pipelineName, m_vertexShader.GetProgramName()));
			m_vertexShader.BindPerPass();
			GL_CHECK(glActiveShaderProgram(m_pipelineName, m_fragmentShader.GetProgramName()));
			m_fragmentShader.BindPerPass();
		}

	protected:
		T m_vertexShader;
		U m_fragmentShader;
		GLuint m_pipelineName;
	};


}