#version 400 core

#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503
#define M_INV_LOG2 1.4426950408889634073599246810019
#define GAMMA 2.2

float g_gamma = GAMMA;
vec2 g_gamma2 = vec2(GAMMA);
vec3 g_gamma3 = vec3(GAMMA);
vec4 g_gamma4 = vec4(GAMMA);

float g_srandSeed = 0.0;
void srand(float seed)
{
	g_srandSeed = seed;
}
float rand()
{
	g_srandSeed = fract(sin(g_srandSeed) * 43758.5453123);
	return g_srandSeed;
}

float degamma( float value )
{
	return pow(value, g_gamma);
}

vec2 degamma( vec2 value )
{
	return pow(value, g_gamma2);
}

vec3 degamma( vec3 value )
{
	return pow(value, g_gamma3);
}

vec4 degamma( vec4 value )
{
	return pow(value, g_gamma4);
}

float packUnorm4x8f( vec4 _value )
{
	return uintBitsToFloat( packUnorm4x8(_value) );
}

vec4 unpackUnorm4x8f( float _value )
{
	return unpackUnorm4x8( floatBitsToUint( _value ) );
}

vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar )
{
	float strength = 1.0;
	vec4 ret = vec4(0.0);
	float totalStrength = 0.0;

	for ( int i = _minMip; i < _maxMip; i++ )
	{
		vec4 samp = textureLod( _sampler, _uv, i );
		ret += samp * strength;
		totalStrength += strength;
		strength *= _downSampleScalar;
	}

	ret /= totalStrength;

	return ret;
}

vec3 decodeNormalDXT( vec4 _sample )
{
	vec3 n = vec3(_sample.w, _sample.x, 0.0) * 2.0 - 1.0;
	n.z = sqrt(1.0-(n.x*n.x + n.y*n.y));
	return n;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TRANSFORMATIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec2 rotateVec2(vec2 vec, float radians)
{
	vec2 result = vec;
	float cosine = cos( radians );
	float sine = sin( radians );
    result.x = vec.x * cosine - vec.y * sine;
    result.y = vec.x * sine + vec.y * cosine;
    return result;
}