#version 330 core

#define M_PI 3.1415926535897932384626433832795
#define M_INV_PI 0.31830988618379067153776752674503

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

vec3 ApplyFog( vec3 rgb, vec3 c, vec3 p, vec3 sunDir, float height, vec3 colorAway, vec3 colorTowards, float density )
{
	vec4 C = vec4(c,1.0);
	vec4 P = vec4(p,1.0);
	vec4 F = vec4(0.0,1.0,0.0,-height);
	vec4 V = C-P;
	float a = density;
	float k = dot(F,C) <= 0.0 ? 1.0 : 0.0;

	float sunDot = max( dot( -normalize(V.xyz), sunDir ), 0.0 );
    vec3  color  = mix( colorAway,
                        colorTowards,
                        pow(sunDot,4.0) );

	vec3 aV = a * V.xyz * 0.5;
	float c1 = k * (dot(F,P) + dot(F,C));
	float c2 = (1.0-2.0*k) * dot(F,P);
	float g = min(c2, 0.0);
	g = -length(aV) * (c1 - g * g / abs(dot(F,V)));
	
	float f = clamp(exp2(-g*g), 0.0, 1.0);

	return mix( color, rgb, f );
} 

////////////////////////////////////////////////////////////////
// Lighting GGX
////////////////////////////////////////////////////////////////

float lightingVis(float dotNV, float k)
{
	return 1.0f/(dotNV*(1.0f-k)+k);
}

float lightingFresnel( float dotNV, float F0 )
{
	float dotNVPow = pow(1.0f-dotNV,3);
	float F = F0 + (1.0-F0)*(dotNVPow);
	return F;
}

float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 )
{
	float alpha = roughness*roughness;

	vec3 H = normalize(V+L);

	float dotNL = clamp(dot(N,L), 0.0f, 1.0f);
	float dotNV = clamp(dot(N,V), 0.0f, 1.0f);
	float dotNH = clamp(dot(N,H), 0.0f, 1.0f);
	float dotLH = clamp(dot(L,H), 0.0f, 1.0f);

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
	float D = alphaSqr/(pi * denom * denom);

	// F
	float F = lightingFresnel(dotNV, F0);

	// V
	float k = alpha/2.0f;
	float vis = lightingVis(dotNL,k)*lightingVis(dotNV,k);

	return min( 10.0f, dotNL * D * F * vis );
}

vec3 lightingGGXAlbedo( vec3 N, vec3 V, vec3 L, float roughness, float F0, vec3 albedo )
{
	float alpha = roughness*roughness;

	vec3 H = normalize(V+L);

	float dotNL = clamp(dot(N,L), 0.0f, 1.0f);
	float dotNV = clamp(dot(N,V), 0.0f, 1.0f);
	float dotNH = clamp(dot(N,H), 0.0f, 1.0f);
	float dotLH = clamp(dot(L,H), 0.0f, 1.0f);
	float dotVH = clamp(dot(V,H), 0.0f, 1.0f);

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
	float D = alphaSqr/(pi * denom * denom);

	// F
	float F = lightingFresnel(dotVH, F0);

	// V
	float k = alpha/2.0f;
	float vis = lightingVis(dotNL,k)*lightingVis(dotNV,k);

	vec3 specResponse = vec3( dotNL * D * F * vis );
	vec3 diffuseResponse = dotNL * albedo;

	return mix( diffuseResponse, specResponse, F );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// use GGX / Trowbridge-Reitz, same as Disney and Unreal 4
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
float normalDistribution
(
	float dotNH,
	float roughness
)
{
	float alpha = roughness * roughness;
	float tmp = alpha / max(1e-8,(dotNH*dotNH*(alpha*alpha-1.0)+1.0));
	return tmp * tmp * M_INV_PI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Schlick with Spherical Gaussian approximation
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 fresnel
(
	float vdh,
	vec3 F0
)
{
	float sphg = pow(2.0, (-5.55473*vdh - 6.98316) * vdh);
	return F0 + (vec3(1.0, 1.0, 1.0) - F0) * sphg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// One generic factor of the geometry function divided by ndw
// NB : We should have k > 0
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
float G1
(
	float ndw, // w is either Ln or Vn
	float k
)
{
	return 1.0 / ( ndw*(1.0-k) + k );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Schlick with Smith-like choice of k
// cf http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p3
// visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
float visibility
(
	float dotNL,
	float dotNV,
	float roughness
)
{
	float k = roughness * roughness * 0.5;
	return G1(dotNL,k) * G1(dotNV,k);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 specularBRDF
(
	vec3 N,
	vec3 L,
	vec3 V,
	vec3 color,
	float roughness
)
{
	vec3 H = normalize(V + L);
	float dotVH = max( 0.0, dot(V, H) );
	float dotNH = max( 0.0, dot(N, H) );
	float dotNL = max( 0.0, dot(N, L) );
	float dotNV = max( 0.0, dot(N, V) );

	return fresnel(dotVH, color) * ( normalDistribution(dotNH, roughness) * visibility(dotNL, dotNV, roughness) / 4.0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 diffuseBRDF
(
	vec3 N,
	vec3 L,
	vec3 V,
	vec3 color
)
{
	return color * M_INV_PI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
vec3 pointLightContribution
(
	vec3 N,
	vec3 L,
	vec3 V,
	vec3 diffColor,
	vec3 specColor,
	float roughness,
	vec3 lightColor,
	float lightIntensity
)
{
	float dp = max( dot( N, L ), 0.0 );
	vec3 diffuse = diffuseBRDF( N, L, V, diffColor * ( vec3(1.0)-specColor ) );
	vec3 spec = specularBRDF( N, L, V, specColor, roughness );

	return dp * ( diffuse + spec ) * lightColor * lightIntensity * M_PI;
}
