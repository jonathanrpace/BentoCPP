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

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataA = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataB = texelFetch(s_heightData, texelCoordR, 0);

	float heatC = heightDataC.z;
	float heatA = heightDataA.z;
	float heatB = heightDataB.z;

	float diffA = heatA - heatC;
	float diffB = heatB - heatC;

	float newHeat = heatC;
	
	newHeat += clamp( diffA*u_heatSmoothStrength, -heatC*u_heatSmoothStrength, heatA*u_heatSmoothStrength );
	newHeat += clamp( diffB*u_heatSmoothStrength, -heatC*u_heatSmoothStrength, heatB*u_heatSmoothStrength );


	// Smooth heat
	//newHeat += clamp((heatA-heatC), -heatA*0.25, heatA*0.25) * u_heatSmoothStrength;
	//newHeat += clamp((heatB-heatC), -heatB*0.25, heatB*0.25) * u_heatSmoothStrength;

	out_heightData = vec4(heightDataC.x, heightDataC.y, newHeat, heightDataC.w);
}





