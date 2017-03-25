#pragma once

#include <map>

#include <bento.h>

#include <bento/core/AbstractValidatable.h>
#include <bento/core/ITexture.h>

namespace bento
{
	class ShaderStageBase 
		: public bento::AbstractValidatable
	{

	public:
		static void StaticInit();
		static void StaticShutdown();

	protected:
		static GLuint s_standardLibShaderNameVert;
		static GLuint s_standardLibShaderNameFrag;
		static GLuint s_activeShaderPipeline;
		static GLuint s_activeShaderProgram;

	public:
		ShaderStageBase(const char* _filename, bool _useSSO = true);
		~ShaderStageBase();

		void SetPipelineName(GLuint _pipelineName);
		GLuint GetProgramName();
		void BindPerPass(int* _textureUnit);
		
		void SetUniform(const char* _name, mat4& _value, bool _transposed = false);
		void SetUniform(const char* _name, mat3& _value, bool _transposed = false);
		void SetUniform(const char* _name, float _value);
		void SetUniform(const char* _name, vec2& _value);
		void SetUniform(const char* _name, vec3& _value);
		void SetUniform(const char* _name, vec4& _value);
		void SetUniform(const char* _name, ivec2& _value);
		void SetUniform(const char* _name, int _value);

		void SetTexture(const char* _name, ITexture& _texture);

	protected:
		void Validate();
		void OnInvalidate();
		virtual void OnPreLink();
		int GetUniformLocation(const char * _name);

		bool m_useSSO;
		const char* m_filename;
		GLuint m_pipelineName;
		GLuint m_programName;
		GLenum m_shaderType;
		int* m_textureUnit;

		std::map<std::string, int> m_uniformMap;

		void SetAsActiveShader();
	};
}