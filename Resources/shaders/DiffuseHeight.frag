#version 330 core

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_waterData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform ivec2 u_axis;

uniform float u_strength = 0.5;

// Outputs
layout( location = 0 ) out vec4 out_waterData;

void main(void)
{
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,1) * u_axis;
	ivec2 texelCoordR = texelCoordC + ivec2(1,1) * u_axis;

	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
	vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);

	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	vec4 waterDataL = texelFetch(s_waterData, texelCoordL, 0);
	vec4 waterDataR = texelFetch(s_waterData, texelCoordR, 0);
	
	float waterHeightC = rockDataC.x + rockDataC.y + rockDataC.w + waterDataC.x + waterDataC.y;
	float waterHeightL = rockDataL.x + rockDataL.y + rockDataL.w + waterDataL.x + waterDataL.y;
	float waterHeightR = rockDataR.x + rockDataR.y + rockDataR.w + waterDataR.x + waterDataR.y;

	float diffL = waterHeightL - waterHeightC;
	float diffR = waterHeightR - waterHeightC;

	float waterVolumeC = waterDataC.y;
	waterVolumeC += clamp(diffL*0.25, -waterDataC.y*0.25, waterDataL.y*0.25);
	waterVolumeC += clamp(diffR*0.25, -waterDataC.y*0.25, waterDataR.y*0.25);

	out_waterData = vec4(waterDataC.x, waterVolumeC, waterDataC.zw);
}





