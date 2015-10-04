#pragma once

#include <fstream>

#include <glew.h>
#include <ramen.h>
#include <kaiga.h>

namespace Kaiga
{
	class AbstractShaderStage :
		public Ramen::AbstractValidatable
	{

	private:
		// STATIC ////////////////////////////////////////////////////////////////
		static unsigned long GetFileLength(std::ifstream& _file);
		static int LoadShader(char* _filename, GLchar** o_ShaderSource, unsigned long* o_len);
		static void UnloadShader(GLubyte** ShaderSource);
		//////////////////////////////////////////////////////////////////////////
	public:
		AbstractShaderStage( char* _filename, int _shaderType );
		~AbstractShaderStage();

		GLuint GetProgramName();
		virtual void BindPerPass() = 0;
		
	protected:

		void Validate();
		void OnInvalidate();

		char* m_filename;
		GLuint m_pipelineName;
		GLuint m_programName;
		GLenum const m_shaderType;
	};
}