#version 330 core



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

float lightingAngularDot( vec3 A, vec3 B, in float angularSize )
{
	float dotValue = clamp( dot( A, B ), 0.0f, 1.0f );
	float pi = 3.14159f;

	float angle = 1.0f - acos(dotValue) * (2.0f / pi);
	angle = min( angle, angularSize );
	angle *= ( 1 / angularSize );

	float ret = sin( angle * pi * 0.5f );
	
	return ret;
}

float lightingFresnel( float dotNV, float F0 )
{
	float dotNVPow = pow(1.0f-dotNV,2);
	float F = F0 + (1.0-F0)*(dotNVPow);
	return F;
}

float lightingGGXAngular( vec3 N, vec3 V, vec3 L, float roughness, float F0, float angularSize )
{
	float alpha = roughness*roughness;

	vec3 H = normalize(V+L);

	float dotNL = lightingAngularDot(N,L,angularSize);
	float dotNV = clamp(dot(N,V), 0.0f, 1.0f);
	float dotNH = lightingAngularDot(N,H,angularSize);
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

vec3 lightingGGXAlbedo( vec3 N, vec3 V, vec3 L, float roughness, float F0, float reflectivity, vec3 albedo )
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

	return mix( diffuseResponse, specResponse, reflectivity );

}
