#pragma once

#include <fstream>

#include <glew.h>
#include <glm.h>
#include <glm/gtc/type_ptr.hpp>

#include <Ramen/Core/AbstractValidatable.h>

#include <Kaiga/Textures/RectangleTexture.h>
#include <Kaiga/Textures/TextureSquare.h>
#include <Kaiga/Core/RenderParams.h>

namespace Kaiga
{
	class ShaderStageBase 
		: public Ramen::AbstractValidatable
	{
		// STATIC 
	private:
		static unsigned long GetFileLength(std::ifstream& _file);
		static int LoadShader(char* _filename, GLchar** o_ShaderSource, unsigned long* o_len);
		static void UnloadShader(GLchar** ShaderSource);
		// ~STATIC

	public:
		ShaderStageBase(char* _filename);
		~ShaderStageBase();

		void SetPipelineName(GLuint _pipelineName);
		GLuint GetProgramName();
		void BindPerPass();
		
	protected:
		void Validate();
		void OnInvalidate();

		void SetUniform(const char* _name, mat4& _value, bool _transposed = false);
		void SetUniform(const char* _name, mat3& _value, bool _transposed = false);
		void SetUniform(const char* _name, float _value);
		void SetUniform(const char* _name, vec2& _value);
		void SetUniform(const char* _name, vec3& _value);
		void SetUniform(const char* _name, vec4& _value);
		void SetUniform(const char* _name, int _value);

		void SetTexture(const char* _name, RectangleTexture* _texture);
		void SetTexture(const char* _name, TextureSquare* _texture);

		char* m_filename;
		GLuint m_pipelineName;
		GLuint m_programName;
		GLenum m_shaderType;
		int m_textureUnit;

	protected:
		void SetAsActiveShader();
	};
}