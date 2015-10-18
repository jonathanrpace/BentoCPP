#pragma once

#include <fstream>

#include <glew.h>
#include <ramen.h>
#include <kaiga.h>
#include <glew.h>

#include <glm/gtc/type_ptr.hpp>

namespace Kaiga
{
	class ShaderStageBase :
		public Ramen::AbstractValidatable
	{
		// STATIC 
	private:
		static unsigned long GetFileLength(std::ifstream& _file);
		static int LoadShader(char* _filename, GLchar** o_ShaderSource, 
			unsigned long* o_len);
		static void UnloadShader(GLchar** ShaderSource);
		// ~STATIC
	public:
		ShaderStageBase( char* _filename );
		~ShaderStageBase();

		void SetPipelineName(GLuint _pipelineName);
		GLuint GetProgramName();
		void BindPerPass();
		
	protected:

		void Validate();
		void OnInvalidate();

		void SetUniformMatrix(const char* name, mat4& matrix, bool transposed = false);
		void SetUniformMatrix(const char* name, mat3& matrix, bool transposed = false);

		char* m_filename;
		GLuint m_pipelineName;
		GLuint m_programName;
		GLenum m_shaderType;

	private:
		void SetAsActiveShader();
	};
}