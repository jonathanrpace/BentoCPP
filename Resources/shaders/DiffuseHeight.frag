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
	
	float newWaterHeight = waterDataC.x;
	/*
	{
		float waterHeightC = waterDataC.x;
		float waterHeightL = waterDataL.x;
		float waterHeightR = waterDataR.x;

		float diffL = (waterHeightL - waterHeightC) * u_waterDiffuseStrength;
		float diffR = (waterHeightR - waterHeightC) * u_waterDiffuseStrength;

		newWaterHeight += clamp(diffL*0.25, -waterDataC.x*0.25, waterDataL.x*0.25);
		newWaterHeight += clamp(diffR*0.25, -waterDataC.x*0.25, waterDataR.x*0.25);
	}
	*/
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

	float newDissolvedDirt = waterDataC.w;
	{
		float dissolvedDirtHeightC = waterDataC.w;
		float dissolvedDirtHeightL = waterDataL.w;
		float dissolvedDirtHeightR = waterDataR.w;

		float diffL = (dissolvedDirtHeightL - dissolvedDirtHeightC) * u_waterDiffuseStrength;
		float diffR = (dissolvedDirtHeightR - dissolvedDirtHeightC) * u_waterDiffuseStrength;

		newDissolvedDirt += clamp(diffL*0.25, -waterDataC.w*0.25, waterDataL.w*0.25);
		newDissolvedDirt += clamp(diffR*0.25, -waterDataC.w*0.25, waterDataR.w*0.25);
	}

	vec2 newWaterVelocity = waterDataC.yz;
	{
		vec2 diffL = (waterDataL.yz - waterDataC.yz) * u_dirtDiffuseStrength;
		vec2 diffR = (waterDataR.yz - waterDataC.yz) * u_dirtDiffuseStrength;

		newWaterVelocity += diffL * 0.5;
		newWaterVelocity += diffR * 0.5;
	}

	out_waterData = vec4(newWaterHeight, newWaterVelocity, newDissolvedDirt);
	out_rockData = vec4(rockDataC.xyz, newDirtHeight);
}





