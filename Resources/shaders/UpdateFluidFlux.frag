#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_elasticity;

uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_fluxData;

void main(void)
{ 
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 flux = texture(s_fluxData, in_uv);

	vec4 heightDataSampleL = texture(s_heightData, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightDataSampleR = texture(s_heightData, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightDataSampleU = texture(s_heightData, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightDataSampleD = texture(s_heightData, in_uv + vec2(0.0f,texelSize.y));

	vec4 terrainHeight = vec4(heightDataSample.x);
	vec4 nTerrainHeight = vec4(heightDataSampleL.x, heightDataSampleR.x, heightDataSampleU.x, heightDataSampleD.x);
	vec4 fluidHeight = vec4(heightDataSample.y);
	vec4 nFluidHeight = vec4(heightDataSampleL.y, heightDataSampleR.y, heightDataSampleU.y, heightDataSampleD.y);

	vec4 height = terrainHeight + fluidHeight;
	vec4 nHeight = nTerrainHeight + nFluidHeight;
	vec4 heightDiff = height - nHeight;

	vec4 newFlux = max(vec4(0.0f), flux + u_elasticity * heightDiff);

	// Need to scale down the new flux so that we can't drain more fluid than we have this step
	float limit = min(1.0f, fluidHeight.x / (newFlux.x + newFlux.y + newFlux.z + newFlux.w));
	newFlux *= limit;

	out_fluxData = newFlux;
}








