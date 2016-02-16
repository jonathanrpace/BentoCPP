#pragma once

#include <fstream>

#include <bento.h>

namespace bento
{
	unsigned long GetFileLength(std::ifstream& _file);
	int LoadShader(char* _filename, GLchar** o_ShaderSource, unsigned long* o_len);
	void UnloadShader(GLchar** _shaderSource);
}