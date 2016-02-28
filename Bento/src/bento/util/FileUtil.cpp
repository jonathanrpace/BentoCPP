#include "FileUtil.h"

#include <iostream>

#include <bento.h>

unsigned long bento::fileUtil::GetFileLength(std::ifstream& _file)
{
	if (!_file.good()) return 0;

	std::streampos pos = _file.tellg();
	_file.seekg(0, std::ios::end);
	std::streampos len = _file.tellg();
	_file.seekg(std::ios::beg);

	return (unsigned long)len;
}

int bento::fileUtil::LoadFile(char* _filename, char** o_fileContents, unsigned long* o_len)
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
	GLchar* shaderSource = *o_shaderSourceHandle;z
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
