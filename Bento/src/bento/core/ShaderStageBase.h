#pragma once

#include <bento.h>

#include <bento/core/AbstractValidatable.h>
#include <bento/render/RenderParams.h>
#include <bento/components/textures/RectangleTexture.h>
#include <bento/components/textures/TextureSquare.h>

namespace bento
{
	class ShaderStageBase 
		: public bento::AbstractValidatable
	{

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

		void SetTexture(const char* _name, RectangleTexture* _texture);
		void SetTexture(const char* _name, TextureSquare* _texture);

	protected:
		void Validate();
		void OnInvalidate();
		virtual void OnPreLink();

		bool m_useSSO;
		const char* m_filename;
		GLuint m_pipelineName;
		GLuint m_programName;
		GLenum m_shaderType;
		int* m_textureUnit;

		void SetAsActiveShader();
	};
}