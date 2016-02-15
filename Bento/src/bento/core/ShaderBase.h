#pragma once

#include <assert.h>

#include <bento\Util\GLErrorUtil.h>

namespace bento
{
	template<typename T, typename U>
	class ShaderBase
	{
	public:
		ShaderBase() :
			m_pipelineName(0),
			m_vertexShader(),
			m_fragmentShader()
		{
			GL_CHECK(glGenProgramPipelines(1, &m_pipelineName));
			GL_CHECK(glUseProgramStages(m_pipelineName, GL_VERTEX_SHADER_BIT, m_vertexShader.GetProgramName()));
			GL_CHECK(glUseProgramStages(m_pipelineName, GL_FRAGMENT_SHADER_BIT, m_fragmentShader.GetProgramName()));
			GL_CHECK(glBindProgramPipeline(GL_NONE));

			m_vertexShader.SetPipelineName(m_pipelineName);
			m_fragmentShader.SetPipelineName(m_pipelineName);
		}
		~ShaderBase()
		{
			GL_CHECK(glDeleteProgramPipelines(1, &m_pipelineName));
		}

		void BindPerPass()
		{
			m_textureUnit = 0;
			assert(glIsProgramPipeline(m_pipelineName));
			GL_CHECK(glBindProgramPipeline(m_pipelineName));
			GL_CHECK(glActiveShaderProgram(m_pipelineName, m_vertexShader.GetProgramName()));
			m_vertexShader.BindPerPass(&m_textureUnit);
			GL_CHECK(glActiveShaderProgram(m_pipelineName, m_fragmentShader.GetProgramName()));
			m_fragmentShader.BindPerPass(&m_textureUnit);
		}

		T& VertexShader()
		{
			return m_vertexShader;
		}

		U& FragmentShader()
		{
			return m_fragmentShader;
		}

	protected:
		T m_vertexShader;
		U m_fragmentShader;
		GLuint m_pipelineName;
		int m_textureUnit;
	};
}