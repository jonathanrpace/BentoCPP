#version 330 core

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_rockFluxDamping;
uniform float u_waterFluxDamping;

// Outputs
layout( location = 0 ) out vec4 out_rockFluxData;
layout( location = 1 ) out vec4 out_waterFluxData;

void main(void)
{ 
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
	vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);
	vec4 rockDataU = texelFetch(s_rockData, texelCoordU, 0);
	vec4 rockDataD = texelFetch(s_rockData, texelCoordD, 0);

	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	vec4 waterDataL = texelFetch(s_waterData, texelCoordL, 0);
	vec4 waterDataR = texelFetch(s_waterData, texelCoordR, 0);
	vec4 waterDataU = texelFetch(s_waterData, texelCoordU, 0);
	vec4 waterDataD = texelFetch(s_waterData, texelCoordD, 0);

	vec4 rockHeightC = vec4(rockDataC.x);
	vec4 dirtHeightC = vec4(rockDataC.w);
	vec4 moltenHeightC = vec4(rockDataC.y);
	vec4 iceHeightC = vec4(waterDataC.x);
	vec4 waterHeightC = vec4(waterDataC.y);
	
	vec4 rockHeightN = vec4(rockDataL.x, rockDataR.x, rockDataU.x, rockDataD.x);
	vec4 dirtHeightN = vec4(rockDataL.w, rockDataR.w, rockDataU.w, rockDataD.w);
	vec4 moltenHeightN = vec4(rockDataL.y, rockDataR.y, rockDataU.y, rockDataD.y);
	vec4 iceHeightN = vec4(waterDataL.x, waterDataR.x, waterDataU.x, waterDataD.x);
	vec4 waterHeightN = vec4(waterDataL.y, waterDataR.y, waterDataU.y, waterDataD.y);

	// Molten flux
	{
		vec4 heightC = rockHeightC + dirtHeightC + moltenHeightC;
		vec4 heightN = rockHeightN + dirtHeightN + moltenHeightN;
		vec4 heightDiff = max( heightC - heightN, vec4(0.0f) );

		vec4 rockFluxC = texelFetch(s_rockFluxData, texelCoordC, 0);
		rockFluxC += heightDiff;
		
		// Need to scale down the new flux so that we can't drain more fluid than we have this step
		float limit = min(1.0f, moltenHeightC.x / (rockFluxC.x + rockFluxC.y + rockFluxC.z + rockFluxC.w + 0.0001f) );
		limit = smoothstep(0,1,limit);
		rockFluxC *= limit;

		rockFluxC *= u_rockFluxDamping;

		out_rockFluxData = rockFluxC;
	}
	// Water flux
	{
		vec4 heightC = rockHeightC + dirtHeightC + moltenHeightC + iceHeightC + waterHeightC;
		vec4 heightN = rockHeightN + dirtHeightN + moltenHeightN + iceHeightN + waterHeightN;
		vec4 heightDiff = max( heightC - heightN, vec4(0.0f) );

		vec4 waterFluxC = texelFetch(s_waterFluxData, texelCoordC, 0);
		waterFluxC += heightDiff;
		
		// Need to scale down the new flux so that we can't drain more fluid than we have this step
		float totalOutflow = waterFluxC.x + waterFluxC.y + waterFluxC.z + waterFluxC.w + 0.0001f;
		float limit = min(1.0f, waterHeightC.x / totalOutflow );
		limit = smoothstep(0,1,limit);
		waterFluxC *= limit;

		waterFluxC *= u_waterFluxDamping;

		out_waterFluxData = waterFluxC;
	}
	
}








