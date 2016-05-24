#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform ivec2 u_axis;
uniform float u_dirtDiffuseStrength;
uniform float u_waterDiffuseStrength;

// Outputs
layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_velocityData;
layout( location = 2 ) out vec4 out_miscData;

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

	out_heightData = heightDataC;
	out_velocityData = velocityDataC;
	out_miscData = miscDataC;

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
		float c = miscDataC.y;
		float l = miscDataL.y;
		float r = miscDataR.y;

		float diffL = (l - c) * u_waterDiffuseStrength;
		float diffR = (r - c) * u_waterDiffuseStrength;

		out_miscData.z += clamp(diffL*0.25, -c*0.25, l*0.25);
		out_miscData.z += clamp(diffR*0.25, -c*0.25, l*0.25);
	}

	// Water velocity
	{
		vec2 diffL = (velocityDataL.zw - velocityDataC.zw) * u_dirtDiffuseStrength;
		vec2 diffR = (velocityDataR.zw - velocityDataC.zw) * u_dirtDiffuseStrength;

		out_velocityData.zw += diffL * 0.5;
		out_velocityData.zw += diffR * 0.5;
	}
}





