#include "TextureUtil.h"

namespace bento
{
	namespace textureUtil
	{
		int GetBestPowerOfTwo(int _value)
		{
			int p = 1;

			while (p < _value)
				p <<= 1;

			if (p > MAX_SIZE) p = MAX_SIZE;

			return p;
		}

		int GetNumMipMaps(int _size)
		{
			_size = GetBestPowerOfTwo(_size);

			if (_size < MIN_SIZE)
				return 0;

			int numMipMaps = 1;
			while (_size > MIN_SIZE)
			{
				_size >>= 1;
				numMipMaps++;
			}

			return numMipMaps - 1;
		}

		int IsPowerOfTwo(int _value)
		{
			return _value > 0 && ((_value & -_value) == _value);
		}

		int IsDimensionValid(int _value)
		{
			return _value >= MIN_SIZE && _value <= MAX_SIZE && IsPowerOfTwo(_value);
		}
	}
}

