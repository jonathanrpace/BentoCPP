#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_smudgeData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform ivec2 u_axis;
uniform float u_dirtDiffuseStrength;
uniform float u_dissolvedDirtDiffuseStrength;
uniform float u_waterDiffuseStrength;
uniform float u_heatDiffuseStrength;

// Outputs
layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_velocityData;
layout( location = 2 ) out vec4 out_miscData;
layout( location = 3 ) out vec4 out_smudgeData;

void main(void)
{
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,1) * u_axis;
	ivec2 texelCoordR = texelCoordC + ivec2(1,1) * u_axis;

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);

	vec4 velocityDataC = texelFetch(s_velocityData, texelCoordC, 0);
	vec4 velocityDataL = texelFetch(s_velocityData, texelCoordL, 0);
	vec4 velocityDataR = texelFetch(s_velocityData, texelCoordR, 0);

	vec4 miscDataC = texelFetch(s_miscData, texelCoordC, 0);
	vec4 miscDataL = texelFetch(s_miscData, texelCoordL, 0);
	vec4 miscDataR = texelFetch(s_miscData, texelCoordR, 0);

	vec4 smudgeDataC = texelFetch(s_smudgeData, texelCoordC, 0);
	vec4 smudgeDataL = texelFetch(s_smudgeData, texelCoordL, 0);
	vec4 smudgeDataR = texelFetch(s_smudgeData, texelCoordR, 0);

	out_heightData = heightDataC;
	out_velocityData = velocityDataC;
	out_miscData = miscDataC;
	out_smudgeData = smudgeDataC;

	// Heat
	{
		float c = miscDataC.x;
		float l = miscDataL.x;
		float r = miscDataR.x;

		float diffL = (l - c) * u_heatDiffuseStrength * 0.5;
		float diffR = (r - c) * u_heatDiffuseStrength * 0.5;

		out_miscData.x += diffL;
		out_miscData.x += diffR;
	}



	/*
	// Smudge
	{
		float c = smudgeDataC.x;
		float l = smudgeDataL.x;
		float r = smudgeDataR.x;

		float diffL = (l - c) * u_heatDiffuseStrength * 0.5;
		float diffR = (r - c) * u_heatDiffuseStrength * 0.5;

		out_miscData.x += diffL;
		out_miscData.x += diffR;
	}
	*/

	
	// Dirt
	{
		float c = heightDataC.x + heightDataC.y + heightDataC.z;
		float l = heightDataL.x + heightDataL.y + heightDataL.z;
		float r = heightDataR.x + heightDataR.y + heightDataR.z;

		float diffL = (l - c) * u_dirtDiffuseStrength;
		float diffR = (r - c) * u_dirtDiffuseStrength;

		out_heightData.z += clamp(diffL*0.25, -heightDataC.z*0.25, heightDataL.z*0.25);
		out_heightData.z += clamp(diffR*0.25, -heightDataC.z*0.25, heightDataR.z*0.25);
	}

	
	// Dissolved dirt
	{
		float c = miscDataC.z;
		float l = miscDataL.z;
		float r = miscDataR.z;

		float diffL = (l - c) * u_dissolvedDirtDiffuseStrength;
		float diffR = (r - c) * u_dissolvedDirtDiffuseStrength;

		out_miscData.z += clamp(diffL*0.25, -c*0.25, l*0.25);
		out_miscData.z += clamp(diffR*0.25, -c*0.25, l*0.25);
	}

	/*
	// Water velocity
	{
		vec2 diffL = (velocityDataL.zw - velocityDataC.zw) * u_dirtDiffuseStrength;
		vec2 diffR = (velocityDataR.zw - velocityDataC.zw) * u_dirtDiffuseStrength;

		out_velocityData.zw += diffL * 0.5;
		out_velocityData.zw += diffR * 0.5;
	}
	*/
}





