#include "ShaderStageBase.h"

#include <iostream>
#include <assert.h>
#include <stdlib.h>

#include <bento.h>
#include <bento/util/FileUtil.h>

namespace bento
{
	GLuint ShaderStageBase::s_standardLibShaderNameVert(0);
	GLuint ShaderStageBase::s_standardLibShaderNameFrag(0);

	void ShaderStageBase::StaticInit()
	{
		s_standardLibShaderNameVert = glCreateShader(GL_VERTEX_SHADER);
		s_standardLibShaderNameFrag = glCreateShader(GL_FRAGMENT_SHADER);

		char* shaderSourcePtr;
		unsigned long shaderSourceLength;
		std::string resolvedFilename = bento::Config::ResourcePath() + std::string("shaders/StandardLib.glsl");
		fileUtil::LoadFile(resolvedFilename.c_str(), &shaderSourcePtr, &shaderSourceLength);

		glShaderSource(s_standardLibShaderNameVert, 1, &shaderSourcePtr, NULL);
		glShaderSource(s_standardLibShaderNameFrag, 1, &shaderSourcePtr, NULL);

		glCompileShader(s_standardLibShaderNameVert);
		glCompileShader(s_standardLibShaderNameFrag);

		delete[] shaderSourcePtr;
	}

	void ShaderStageBase::StaticShutdown()
	{
		glDeleteShader(s_standardLibShaderNameVert);
		glDeleteShader(s_standardLibShaderNameFrag);
	}


	//************************************
	// Method:    ctor
	// Parameter: char * _filename
	// Parameter: int _shaderType
	//************************************
	ShaderStageBase::ShaderStageBase
	(
		const char * _filename,
		bool _useSSO
	) :
		m_filename(_filename),
		m_useSSO(_useSSO),
		m_pipelineName(-1),
		m_programName(-1)
	{
		bool isVertex = std::strstr(m_filename, ".vert") != nullptr;
		bool isFragment = std::strstr(m_filename, ".frag") != nullptr;
		assert(isVertex || isFragment);
		m_shaderType = isVertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
	}


	//************************************
	// Method:    dtor
	//************************************
	ShaderStageBase::~ShaderStageBase()
	{

	}

	void ShaderStageBase::SetPipelineName(GLuint _pipelineName)
	{
		m_pipelineName = _pipelineName;
	}

	//************************************
	// Method:    GetProgramName
	// Access:    public 
	// Returns:   GLuint
	//************************************
	GLuint ShaderStageBase::GetProgramName()
	{
		ValidateNow();
		return m_programName;
	}

	void ShaderStageBase::Validate()
	{
		assert(glIsProgram(m_programName) == false);

		assert(m_shaderType == GL_VERTEX_SHADER ||
				m_shaderType == GL_FRAGMENT_SHADER);

		char* shaderSourcePtr;
		unsigned long shaderSourceLength;
		std::string resolvedFilename = bento::Config::ResourcePath() + std::string(m_filename);
		fileUtil::LoadFile(resolvedFilename.c_str(), &shaderSourcePtr, &shaderSourceLength);
		TRACE("Compiling Shader : ");
		TRACE(m_filename);
		TRACE("\n");

		m_programName = glCreateProgram();
		GLuint shader = glCreateShader(m_shaderType);
		glShaderSource(shader, 1, &shaderSourcePtr, NULL);
		glCompileShader(shader);
		glProgramParameteri(m_programName, GL_PROGRAM_SEPARABLE, GL_TRUE);

		glAttachShader(m_programName, m_shaderType == GL_VERTEX_SHADER ? s_standardLibShaderNameVert : s_standardLibShaderNameFrag);
		glAttachShader(m_programName, shader);

		// Give derived classes the ability to configure any more shader state before linking occurs
		// Primarily used for configuring transform feedback varyings, which is required to happen before a link
		this->OnPreLink();

		glLinkProgram(m_programName);

		CHECK_SHADER_COMPILATION(m_programName);

		glDetachShader(m_programName, shader);
		glDeleteShader(shader);

		delete[] shaderSourcePtr;
	}

	void ShaderStageBase::OnInvalidate()
	{
		if (glIsProgram(m_programName))
		{
			GL_CHECK(glDeleteProgram(m_programName));
			m_programName = GL_NONE;
		}
	}

	void ShaderStageBase::SetUniform(const char * _name, mat4 & _value, bool _transposed)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GLfloat* data = (GLfloat*)glm::value_ptr(_value);
		GL_CHECK(glUniformMatrix4fv(location, 1, _transposed, data));
	}

	void ShaderStageBase::SetUniform(const char * _name, mat3 & _value, bool _transposed)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GLfloat* data = (GLfloat*)glm::value_ptr(_value);
		GL_CHECK(glUniformMatrix3fv(location, 1, _transposed, data));
	}

	void ShaderStageBase::SetUniform(const char * _name, float _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform1f(location, _value));
	}

	void ShaderStageBase::SetUniform(const char * _name, vec2& _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform2f(location, _value.x, _value.y));
	}

	void ShaderStageBase::SetUniform(const char * _name, vec3& _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform3f(location, _value.x, _value.y, _value.z));
	}

	void ShaderStageBase::SetUniform(const char * _name, vec4& _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform4f(location, _value.x, _value.y, _value.z, _value.w));
	}

	void ShaderStageBase::SetUniform(const char * _name, ivec2& _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform2i(location, _value.x, _value.y));
	}

	void ShaderStageBase::SetUniform(const char * _name, int _value)
	{
		SetAsActiveShader();
		GLint location = -1;
		GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
		// assert(location != -1);
		GL_CHECK(glUniform1i(location, _value));
	}

	void ShaderStageBase::SetTexture(const char * _name, TextureSquare& _texture)
	{
		SetAsActiveShader();
		SetUniform(_name, *m_textureUnit);
		glActiveTexture(GL_TEXTURE0 + *m_textureUnit);
		glBindTexture(GL_TEXTURE_2D, _texture.TextureName());
		*(m_textureUnit) = *(m_textureUnit)+1;
	}

	void ShaderStageBase::SetTexture(const char * _name, RectangleTexture& _texture)
	{
		SetAsActiveShader();
		SetUniform(_name, *m_textureUnit);
		glActiveTexture(GL_TEXTURE0 + *m_textureUnit);
		glBindTexture(GL_TEXTURE_RECTANGLE, _texture.TextureName());
		*(m_textureUnit) = *(m_textureUnit)+1;
	}

	void ShaderStageBase::SetTexture(const char * _name, Texture3D& _texture)
	{
		SetAsActiveShader();
		SetUniform(_name, *m_textureUnit);
		glActiveTexture(GL_TEXTURE0 + *m_textureUnit);
		glBindTexture(GL_TEXTURE_3D, _texture.TextureName());
		*(m_textureUnit) = *(m_textureUnit)+1;
	}

	void ShaderStageBase::SetAsActiveShader()
	{
		GL_CHECK(glActiveShaderProgram(m_pipelineName, m_programName);)
	}

	void ShaderStageBase::BindPerPass(int* _textureUnit)
	{
		m_textureUnit = _textureUnit;
	}

	void ShaderStageBase::OnPreLink()
	{
		// Intentionally blank
	}
}