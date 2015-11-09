#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;
uniform sampler2D s_velocityData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_viscocity;
uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void main(void)
{ 
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 flux = texture(s_fluxData, in_uv);
	vec4 velocityDataSample = texture(s_velocityData, in_uv);

	float fluidHeight = heightDataSample.y;

	float fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y)).z;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);

	float fluxChange = ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(flux.x+flux.y+flux.z+flux.w)) * u_viscocity;
	float newFluidHeight = fluidHeight + fluxChange;

	// Add some fluid near the mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	mouseRatio = pow(mouseRatio, 0.5f);
	newFluidHeight += mouseRatio * u_mouseStrength;

	out_heightData = vec4(heightDataSample.x, newFluidHeight, heightDataSample.zw);
}








