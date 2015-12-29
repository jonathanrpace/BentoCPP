#version 330 core

// Samplers
uniform sampler2D s_heightData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform ivec2 u_axis;
uniform float u_heatSmoothStrength;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void main(void)
{
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,1) * u_axis;
	ivec2 texelCoordR = texelCoordC + ivec2(1,1) * u_axis;

	vec4 heightC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightA = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightB = texelFetch(s_heightData, texelCoordR, 0);

	float heat = heightC.z;
	float newHeat = heightC.z;
	
	// Smooth heat
	newHeat += clamp((heightA.z-heat), -heightA.z*0.25, heightA.z*0.25) * u_heatSmoothStrength;
	newHeat += clamp((heightB.z-heat), -heightB.z*0.25, heightB.z*0.25) * u_heatSmoothStrength;

	out_heightData = vec4(heightC.x, heightC.y, newHeat, heightC.w);
}





