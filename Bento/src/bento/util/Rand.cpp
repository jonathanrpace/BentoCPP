#include "Rand.h"

#include <random>

namespace bento
{
	float Rand()
	{
		return static_cast <float> (std::rand()) / static_cast <float> (RAND_MAX);
	}

	float Rand(float _min, float _max)
	{
		return _min + Rand() * (-_max - _min);
	}
}