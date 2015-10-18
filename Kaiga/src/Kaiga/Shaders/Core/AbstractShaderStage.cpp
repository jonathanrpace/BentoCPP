#include "AbstractShaderStage.h"

#include <iostream>
#include <assert.h>

// STATIC
unsigned long Kaiga::AbstractShaderStage::GetFileLength
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

int Kaiga::AbstractShaderStage::LoadShader
(
	char* _filename, 
	GLchar** o_shaderSourceHandle, 
	unsigned long* o_len
)
{
	std::ifstream file;
	file.open(_filename, std::ios::in); // opens as ASCII!
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

void Kaiga::AbstractShaderStage::UnloadShader(GLchar** _shaderSource)
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
Kaiga::AbstractShaderStage::AbstractShaderStage
(
	char * _filename, 
	int _shaderType
) :	
	m_filename(_filename),
	m_shaderType(_shaderType),
	m_pipelineName(-1),
	m_programName(-1)
{
	
}


//************************************
// Method:    dtor
//************************************
Kaiga::AbstractShaderStage::~AbstractShaderStage()
{

}

void Kaiga::AbstractShaderStage::SetPipelineName(GLuint _pipelineName)
{
	m_pipelineName = _pipelineName;
}

//************************************
// Method:    GetProgramName
// Access:    public 
// Returns:   GLuint
//************************************
GLuint Kaiga::AbstractShaderStage::GetProgramName()
{
	ValidateNow();
	return m_programName;
}

void Kaiga::AbstractShaderStage::Validate()
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
	GL_CHECK(m_programName = glCreateShaderProgramEXT(m_shaderType, shaderSourcePtr));
	UnloadShader(&shaderSourcePtr);

	CHECK_SHADER_COMPILATION(m_programName);
}

void Kaiga::AbstractShaderStage::OnInvalidate()
{
	if ( glIsProgram(m_programName) )
	{
		GL_CHECK(glDeleteProgram(m_programName));
		m_programName = GL_NONE;
	}
}


void Kaiga::AbstractShaderStage::SetUniformMatrix(const char * _name, mat4 & _matrix, bool _transposed)
{
	SetAsActiveShader();

	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name);)
	GLfloat* data = (GLfloat*)glm::value_ptr(_matrix);
	GL_CHECK(glUniformMatrix4fv(location, 1, _transposed, data);)
}

void Kaiga::AbstractShaderStage::SetUniformMatrix(const char * _name, mat3 & _matrix, bool _transposed)
{
	SetAsActiveShader();

	GLint location = -1;
	GL_CHECK(location = glGetUniformLocation(m_programName, (GLchar*)_name);)
	GL_CHECK(glUniformMatrix3fv(location, 1, _transposed, glm::value_ptr(_matrix));)
}

void Kaiga::AbstractShaderStage::SetAsActiveShader()
{
	GL_CHECK(glActiveShaderProgram(m_pipelineName, m_programName);)
}

void Kaiga::AbstractShaderStage::BindPerPass()
{
	// Intentionally blank
}