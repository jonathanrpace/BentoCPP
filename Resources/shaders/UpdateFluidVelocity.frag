#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_fluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_textureScrollSpeed;
uniform float u_mapHeightOffset;
uniform float u_velocityScalar;
uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform vec2 u_cellSize;
uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;
layout( location = 2 ) out vec4 out_normal;

float CalcViscosity( float _heat, float _viscosityScalar )
{
	float viscosity = mix( u_viscosityMin, u_viscosityMax, _viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f)), u_heatViscosityPower) * viscosity;
}

void main(void)
{
	ivec2 dimensions = textureSize(s_mappingData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightSample = texture(s_heightData, in_uv);
	vec4 heightSampleL = texture(s_heightData, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightSampleR = texture(s_heightData, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightSampleU = texture(s_heightData, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightSampleD = texture(s_heightData, in_uv + vec2(0.0f,texelSize.y));

	vec4 flux = texture(s_fluxData, in_uv);
	float fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y)).z;

	vec4 mappingData = texture(s_mappingData, in_uv);
	vec4 mappingL = texture(s_mappingData, in_uv - vec2(texelSize.x,0.0f));
	vec4 mappingR = texture(s_mappingData, in_uv + vec2(texelSize.x,0.0f));
	vec4 mappingU = texture(s_mappingData, in_uv - vec2(0.0f,texelSize.y));
	vec4 mappingD = texture(s_mappingData, in_uv + vec2(0.0f,texelSize.y));

	vec4 diffuseSample = texture(s_diffuseMap, mappingData.xy);
	vec4 diffuseSampleL = texture(s_diffuseMap, mappingL.xy);
	vec4 diffuseSampleR = texture(s_diffuseMap, mappingR.xy);
	vec4 diffuseSampleU = texture(s_diffuseMap, mappingU.xy);
	vec4 diffuseSampleD = texture(s_diffuseMap, mappingD.xy);

	float heat = heightSample.z;
	float viscosity = CalcViscosity(heat, diffuseSample.y);

	// Calculate velocity from flux
	vec4 velocity = vec4(0.0f);
	velocity.x = ((fluxL + flux.y) - (fluxR + flux.x)) / texelSize.x;
	velocity.y = ((fluxU + flux.w) - (fluxD + flux.z)) / texelSize.y;
	velocity.xy *= u_velocityScalar * viscosity;

	/*
	vec4 heightDiffs = vec4(0.0f);
	heightDiffs.x = (heightSampleL.x+heightSampleL.y) - (heightSample.x+heightSample.y);
	heightDiffs.y = (heightSampleR.x+heightSampleR.y) - (heightSample.x+heightSample.y);
	heightDiffs.z = (heightSampleU.x+heightSampleU.y) - (heightSample.x+heightSample.y);
	heightDiffs.w = (heightSampleD.x+heightSampleD.y) - (heightSample.x+heightSample.y);

	vec4 h = vec4(0.0f);
	h.x = length( vec2(heightDiffs.x, u_cellSize.x) );
	h.y = length( vec2(heightDiffs.y, u_cellSize.x) );
	h.z = length( vec2(heightDiffs.z, u_cellSize.y) );
	h.w = length( vec2(heightDiffs.w, u_cellSize.y) );
	velocity.xy /= 1.0f + (h.x+h.y+h.z+h.w);
	*/

	// Update mappingData from velocity
	mappingData.xy -= (velocity.xy * u_textureScrollSpeed * 0.01f);

	// Reset mapping near mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	if ( mouseRatio > 0.8f && u_mouseStrength > 0.0f )
	{
		//mappingData.xy = in_uv;
	}

	// Calculate normals
	float heightR = heightSampleR.x + heightSampleR.y;
	float heightL = heightSampleL.x + heightSampleL.y;
	float heightU = heightSampleU.x + heightSampleU.y;
	float heightD = heightSampleD.x + heightSampleD.y;
	float height = heightSample.x + heightSample.y;

	vec3 va = normalize(vec3(u_cellSize.x, 
	(heightR+diffuseSampleR.x*u_mapHeightOffset)-
	(heightL+diffuseSampleL.x*u_mapHeightOffset), 0.0f));

    vec3 vb = normalize(vec3(0.0f, 
	(heightD+diffuseSampleD.x*u_mapHeightOffset)-
	(heightU+diffuseSampleU.x*u_mapHeightOffset), u_cellSize.y));
    vec3 normal = -cross(va,vb);
	
	// Output
	out_velocityData = velocity;
	out_mappingData = mappingData;
	out_normal = vec4(normal,0.0f);
}








