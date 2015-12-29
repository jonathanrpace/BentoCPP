#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_elasticity;
uniform float u_fluxDamping;

// Outputs
layout( location = 0 ) out vec4 out_fluxData;

void main(void)
{ 
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_heightData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_heightData, texelCoordD, 0);

	vec4 fluxC = texelFetch(s_fluxData, texelCoordC, 0);
	vec4 terrainHeightC = vec4(heightDataC.x);
	vec4 fluidHeightC = vec4(heightDataC.y);

	vec4 terrainHeightN = vec4(heightDataL.x, heightDataR.x, heightDataU.x, heightDataD.x);
	vec4 fluidHeightN = vec4(heightDataL.y, heightDataR.y, heightDataU.y, heightDataD.y);
	vec4 heatN = vec4(heightDataL.z, heightDataR.z, heightDataU.z, heightDataD.z);

	vec4 heightC = terrainHeightC + fluidHeightC;
	vec4 heightN = terrainHeightN + fluidHeightN;
	vec4 heightDiff = max( heightC - heightN, vec4(0.0f) );

	vec4 newFlux = fluxC + heightDiff * u_elasticity;
	//newFlux = max( vec4(0.0f), newFlux );
	newFlux *= u_fluxDamping;

	// Need to scale down the new flux so that we can't drain more fluid than we have this step
	float limit = min(1.0f, fluidHeightC.x / (newFlux.x + newFlux.y + newFlux.z + newFlux.w + 0.0001f) * 2.0f);
	newFlux *= limit;

	out_fluxData = newFlux;
}








