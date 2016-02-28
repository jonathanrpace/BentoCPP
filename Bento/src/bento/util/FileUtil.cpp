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

int bento::fileUtil::LoadFile(std::string _filename, char** o_fileContentsHandle, unsigned long* o_len)
{
	std::ifstream file;

	std::string resolvedFilename = bento::Config::ResourcePath() + _filename;
	file.open(resolvedFilename.c_str(), std::ios::in); // opens as ASCII!

	if (!file) 
		return -1;	// Error: Can't open file

	unsigned long fileLength = GetFileLength(file);
	if (fileLength == 0)
		return -2;   // Error: Empty File 

	char* fileContents = new char[fileLength + 1];
	if (fileContents == 0) 
		return -3;   // can't reserve memory

	// len isn't always strlen cause some characters are stripped in 
	// ascii read... it is important to 0-terminate the real length
	// later, len is just max possible value... 
	fileContents[fileLength] = 0;

	unsigned int i = 0;
	while (file.good())
	{
		// get character from file.
		fileContents[i] = file.get();
		if (!file.eof())
			i++;
	}

	fileContents[i] = 0;  // 0-terminate it at the correct position
	file.close();

	*o_len = fileLength;
	*o_fileContentsHandle = fileContents;

	return 0; // No Error
}
