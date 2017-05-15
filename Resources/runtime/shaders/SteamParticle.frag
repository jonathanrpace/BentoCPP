#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in float in_lifeNrm;
in float in_alpha;
in vec3 in_viewPos;
in vec3 in_worldPos;
in float in_alive;

uniform sampler2DRect s_positionBuffer;
uniform sampler2DRect s_outputBuffer;
uniform sampler2D s_texture;
uniform samplerCube s_envMap;
uniform sampler2DRect s_blurredColorBuffer;

uniform vec3 u_albedo;
uniform vec3 u_filterColor;
uniform float u_filterStrength;
uniform float u_density;
uniform float u_ambientLightIntensity;
uniform vec3 u_cameraPos;
uniform vec3 u_cameraForward;
uniform vec3 u_cameraRight;
uniform vec3 u_cameraUp;

////////////////////////////////
// Functions
////////////////////////////////
void srand(float _seed);
float rand();

vec3 rotateX( vec3 _dir, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec3
	( 
		_dir.x, 
		_dir.y * cosValue - _dir.z * sinValue, 
		_dir.y * sinValue + _dir.z * cosValue
	);
}

vec3 rotateZ( vec3 _dir, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec3
	( 
		_dir.x * cosValue - _dir.y * sinValue, 
		_dir.x * sinValue + _dir.y * cosValue,
		_dir.z
	);
}

void main(void)
{
	if ( in_alive < 0.5 )
		discard;

	srand(gl_FragCoord.x * gl_FragCoord.y * in_worldPos.y);

	vec2 uv = gl_PointCoord;
	vec4 textureSample = texture( s_texture, uv );

	vec3 eyeVec = normalize(u_cameraPos-in_worldPos);

	vec3 normal = eyeVec;
	textureSample.rg *= 2.0;
	textureSample.rg -= 1.0;
	normal = rotateX( normal, textureSample.g ); 
	normal = rotateZ( normal, -textureSample.r ); 
	normal = normalize(normal);

	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	float viewDepth = abs( in_viewPos.z - targetViewPosition.z );
	float blendValue = min( 1.0, viewDepth / 0.01f );

	vec3 filteredSample = texelFetch(s_blurredColorBuffer, ivec2(gl_FragCoord.xy * 0.5)).rgb;
	vec4 envMapSample = texture( s_envMap, normal ) * u_ambientLightIntensity;
	vec3 diffuse = u_albedo * envMapSample.rgb * textureSample.b;

	float filterDensity = 1.0 - min( pow(textureSample.a, 1.0/u_density), 1.0 );
	vec3 filtered = filteredSample.rgb * u_filterColor * filterDensity * u_filterStrength;

	float alpha = in_alpha * u_density * blendValue * (textureSample.a+max(filterDensity-1.0, 0.0));

	alpha = clamp(alpha, 0.0, 1.0);
	out_fragColor = vec4(vec3(diffuse+filtered), alpha);
} 