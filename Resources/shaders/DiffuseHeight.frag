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

uniform float u_dirtDiffuseStrength;
uniform float u_waterDiffuseStrength;

// Outputs
layout( location = 0 ) out vec4 out_waterData;
layout( location = 1 ) out vec4 out_rockData;

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
	
	{
		float waterHeightC = waterDataC.x;
		float waterHeightL = waterDataL.x;
		float waterHeightR = waterDataR.x;

		float diffL = (waterHeightL - waterHeightC) * u_waterDiffuseStrength;
		float diffR = (waterHeightR - waterHeightC) * u_waterDiffuseStrength;

		float waterVolumeC = waterDataC.x;
		waterVolumeC += clamp(diffL*0.25, -waterDataC.x*0.25, waterDataL.x*0.25);
		waterVolumeC += clamp(diffR*0.25, -waterDataC.x*0.25, waterDataR.x*0.25);

		out_waterData = vec4(waterVolumeC, waterDataC.yzw);
	}

	float newDirtHeight = rockDataC.w;
	{
		float dirtHeightC = rockDataC.x + rockDataC.y + rockDataC.w;
		float dirtHeightL = rockDataL.x + rockDataL.y + rockDataL.w;
		float dirtHeightR = rockDataR.x + rockDataR.y + rockDataR.w;

		float diffL = (dirtHeightL - dirtHeightC) * u_dirtDiffuseStrength;
		float diffR = (dirtHeightR - dirtHeightC) * u_dirtDiffuseStrength;

		newDirtHeight += clamp(diffL*0.25, -rockDataC.w*0.25, rockDataL.w*0.25);
		newDirtHeight += clamp(diffR*0.25, -rockDataC.w*0.25, rockDataR.w*0.25);
	}
	float newRockHeight =rockDataC.x;
	/*
	
	{
		float heightC = rockDataC.x;
		float heightL = rockDataL.x;
		float heightR = rockDataR.x;

		float diffL = (heightL - heightC) * u_rockDiffuseStrength;
		float diffR = (heightR - heightC) * u_rockDiffuseStrength;

		newRockHeight += clamp(diffL*0.25, -rockDataC.x*0.25, rockDataL.x*0.25);
		newRockHeight += clamp(diffR*0.25, -rockDataC.x*0.25, rockDataR.x*0.25);
	}
	*/
	out_rockData = vec4(newRockHeight, rockDataC.yz, newDirtHeight);
}





