#pragma once

#include <fstream>

namespace bento
{
	namespace fileUtil
	{
		unsigned long GetFileLength(std::ifstream& _file);
		int LoadFile(const char* _filename, char** o_fileContentsHandle, unsigned long* o_len);
		int SaveFile(const char* _filename, const char* _fileContents, unsigned long _size);
	}
}