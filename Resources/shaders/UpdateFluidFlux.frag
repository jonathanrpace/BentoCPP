#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_dt = 1.0f / 4.f;

uniform vec2 u_mousePos;
uniform float u_mouseRadius = 0.05f;
uniform float u_mouseStrength = 0.00f;
uniform float u_mouseFluxStrength = 0.1f;

// Outputs
layout( location = 0 ) out vec4 out_fluxData;

void main(void)
{ 
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 flux = texture(s_fluxData, in_uv);

	// Add some flux away from where we're adding volume
	vec2 mouseOffset = in_uv-u_mousePos;

	float mouseRatio = 1.0f - min(1.0f, length(mouseOffset) / u_mouseRadius);
	mouseOffset = normalize(mouseOffset);
	//flux.x -= mouseOffset.x * mouseRatio * u_mouseFluxStrength * u_mouseStrength;
	//flux.y += mouseOffset.x * mouseRatio * u_mouseFluxStrength * u_mouseStrength;
	//flux.z -= mouseOffset.y * mouseRatio * u_mouseFluxStrength * u_mouseStrength;
	//flux.w += mouseOffset.y * mouseRatio * u_mouseFluxStrength * u_mouseStrength;

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

	vec4 newFlux = max(vec4(0.0f), flux + u_dt * heightDiff);

	// Need to scale down the new flux so that we can't drain more fluid than we have this step
	float limit = min(1.0f, fluidHeight.x / (newFlux.x + newFlux.y + newFlux.z + newFlux.w) );
	newFlux *= limit;

	out_fluxData = newFlux;
}








