#pragma once

#include <fstream>

namespace bento
{
	namespace fileUtil
	{
		unsigned long GetFileLength(std::ifstream& _file);
		int LoadFile(std::string _filename, char** o_fileContentsHandle, unsigned long* o_len);
	}
}