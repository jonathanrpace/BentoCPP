#include "ShaderStageBase.h"

#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <bento.h>

// STATIC
unsigned long bento::ShaderStageBase::GetFileLength
(
	std::ifstream& _file
)
{
	if (!_file.good()) return 0;

	std::streampos pos = _file.tellg();
	_file.seekg(0, std::ios::end);
	std::streampos len = _file.tellg();
	_file.seekg(std::ios::beg);

	return (unsigned long)len;
}

int bento::ShaderStageBase::LoadShader
(
	char* _filename, 
	GLchar** o_shaderSourceHandle, 
	unsigned long* o_len
)
{
	std::ifstream file;

	size_t resolvedFilenameSize = strlen(_filename) + strlen(bento::Config::ResourcePath()) + 1;
	char* resolvedFilename = new char[resolvedFilenameSize];
	strcpy_s(resolvedFilename, resolvedFilenameSize, bento::Config::ResourcePath());
	strcat_s(resolvedFilename, resolvedFilenameSize, _filename);
	file.open(resolvedFilename, std::ios::in); // opens as ASCII!
	delete resolvedFilename;
	if (!file) return -1;

	*o_len = GetFileLength(file);

	if (o_len == 0) return -2;   // Error: Empty File 
	
	*o_shaderSourceHandle = (GLchar*) new char[(*o_len) + 1];
	GLchar* shaderSource = *o_shaderSourceHandle;
	if (shaderSource == 0) return -3;   // can't reserve memory

	// len isn't always strlen cause some characters are stripped in 
	// ascii read... it is important to 0-terminate the real length
	// later, len is just max possible value... 
	shaderSource[*o_len] = 0;

	unsigned int i = 0;
	while (file.good())
	{
		// get character from file.
		shaderSource[i] = file.get();
		if (!file.eof())
			i++;
	}

	shaderSource[i] = 0;  // 0-terminate it at the correct position

	file.close();

	return 0; // No Error
}

void bento::ShaderStageBase::UnloadShader(GLchar** _shaderSource)
{
	if (*_shaderSource != 0)
		delete[] * _shaderSource;
	*_shaderSource = 0;
}
// ~STATIC


//************************************
// Method:    ctor
// Parameter: char * _filename
// Parameter: int _shaderType
//************************************
bento::ShaderStageBase::ShaderStageBase
(
	char * _filename
) :
	m_filename(_filename),
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
bento::ShaderStageBase::~ShaderStageBase()
{

}

void bento::ShaderStageBase::SetPipelineName(GLuint _pipelineName)
{
	m_pipelineName = _pipelineName;
}

//************************************
// Method:    GetProgramName
// Access:    public 
// Returns:   GLuint
//************************************
GLuint bento::ShaderStageBase::GetProgramName()
{
	ValidateNow();
	return m_programName;
}

void bento::ShaderStageBase::Validate()
{
	assert(glIsProgram(m_programName) == false);

	assert(	m_shaderType == GL_VERTEX_SHADER || 
			m_shaderType == GL_FRAGMENT_SHADER );

	GLchar* shaderSourcePtr;
	unsigned long shaderSourceLength;
	LoadShader(m_filename, &shaderSourcePtr, &shaderSourceLength);
	TRACE("Compiling Shader : ");
	TRACE(m_filename);
	TRACE("\n");
	OnPreCompileAndLink();
	GL_CHECK(m_programName = glCreateShaderProgramEXT(m_shaderType, shaderSourcePtr));
	UnloadShader(&shaderSourcePtr);
	OnPostCompileAndLink();

	CHECK_SHADER_COMPILATION(m_programName);
}

void bento::ShaderStageBase::OnInvalidate()
{
	if ( glIsProgram(m_programName) )
	{
		GL_CHECK(glDeleteProgram(m_programName));
		m_programName = GL_NONE;
	}
}

void bento::ShaderStageBase::SetUniform(const char * _name, mat4 & _value, bool _transposed)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GLfloat* data = (GLfloat*)glm::value_ptr(_value);
	GL_CHECK(glUniformMatrix4fv(location, 1, _transposed, data));
}

void bento::ShaderStageBase::SetUniform(const char * _name, mat3 & _value, bool _transposed)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GLfloat* data = (GLfloat*)glm::value_ptr(_value);
	GL_CHECK(glUniformMatrix3fv(location, 1, _transposed, data));
}

void bento::ShaderStageBase::SetUniform(const char * _name, float _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform1f(location, _value));
}

void bento::ShaderStageBase::SetUniform(const char * _name, vec2& _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform2f(location, _value.x, _value.y));
}

void bento::ShaderStageBase::SetUniform(const char * _name, vec3& _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform3f(location, _value.x, _value.y, _value.z));
}

void bento::ShaderStageBase::SetUniform(const char * _name, vec4& _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform4f(location, _value.x, _value.y, _value.z, _value.w));
}

void bento::ShaderStageBase::SetUniform(const char * _name, ivec2& _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform2i(location, _value.x, _value.y));
}

void bento::ShaderStageBase::SetUniform(const char * _name, int _value)
{
	SetAsActiveShader();
	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name));
	// assert(location != -1);
	GL_CHECK(glUniform1i(location, _value));
}

void bento::ShaderStageBase::SetTexture(const char * _name, TextureSquare * _texture)
{
	SetAsActiveShader();
	SetUniform(_name, *m_textureUnit);
	glActiveTexture(GL_TEXTURE0 + *m_textureUnit);
	glBindTexture(GL_TEXTURE_2D, _texture->TextureName());
	*(m_textureUnit) = *(m_textureUnit) + 1;
}

void bento::ShaderStageBase::SetTexture(const char * _name, RectangleTexture * _texture)
{
	SetAsActiveShader();
	SetUniform(_name, *m_textureUnit);
	glActiveTexture(GL_TEXTURE0 + *m_textureUnit);
	glBindTexture(GL_TEXTURE_RECTANGLE, _texture->TextureName());
	*(m_textureUnit) = *(m_textureUnit)+1;
}

void bento::ShaderStageBase::SetAsActiveShader()
{
	GL_CHECK(glActiveShaderProgram(m_pipelineName, m_programName);)
}

void bento::ShaderStageBase::BindPerPass(int* _textureUnit)
{
	m_textureUnit = _textureUnit;
}

void bento::ShaderStageBase::OnPreCompileAndLink()
{
	// Intentionally blank
}

void bento::ShaderStageBase::OnPostCompileAndLink()
{
	// Intentionally blank
}