#pragma once

namespace bento
{
	namespace textureUtil
	{
		static const int MAX_SIZE = 2048;
		static const int MIN_SIZE = 4;

		int GetBestPowerOfTwo(int _value);
		int GetNumMipMaps(int _size);
		int IsPowerOfTwo(int _value);
		int IsDimensionValid(int _value);
	};
}
