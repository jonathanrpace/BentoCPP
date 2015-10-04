#include "AbstractShaderStage.h"

#include <iostream>
#include <assert.h>

// STATIC ////////////////////////////////////////////////////////////////
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

void Kaiga::AbstractShaderStage::UnloadShader(GLubyte** _shaderSource)
{
	if (*_shaderSource != 0)
		delete[] * _shaderSource;
	*_shaderSource = 0;
}
//////////////////////////////////////////////////////////////////////////

Kaiga::AbstractShaderStage::AbstractShaderStage
(
	char * _filename, 
	int _shaderType
) :	
	m_filename(_filename),
	m_shaderType(_shaderType)
{
	Invalidate();
}

Kaiga::AbstractShaderStage::~AbstractShaderStage()
{

}

GLuint Kaiga::AbstractShaderStage::GetProgramName()
{
	ValidateNow();
	return m_programName;
}

void Kaiga::AbstractShaderStage::Validate()
{
	assert(glIsProgram(m_programName) == false);

	assert( m_shaderType == GL_VERTEX_SHADER || 
			m_shaderType == GL_FRAGMENT_SHADER );

	GLchar* shaderSourcePtr;
	unsigned long shaderSourceLength;
	LoadShader(m_filename, &shaderSourcePtr, &shaderSourceLength);

	GL_CHECK(m_programName = glCreateShaderProgramEXT(m_shaderType, shaderSourcePtr));

	GLint infoLogLength;												
	glGetProgramiv((m_programName), GL_INFO_LOG_LENGTH, &infoLogLength);		
	GLchar* infoLog = new GLchar[infoLogLength];						
	glGetProgramInfoLog((m_programName), infoLogLength, nullptr, infoLog);
	std::cout << infoLog; 
	delete infoLog; 
}

void Kaiga::AbstractShaderStage::OnInvalidate()
{
	if ( glIsProgram(m_programName) )
	{
		GL_CHECK(glDeleteProgram(m_programName));
		m_programName = GL_NONE;
	}
}
