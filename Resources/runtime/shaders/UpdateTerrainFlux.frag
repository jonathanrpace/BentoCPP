#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_waterFluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_waterFluxDamping;

// Outputs
layout( location = 0 ) out vec4 out_waterFluxData;

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

	vec4 rockHeightC   = vec4(heightDataC.x);
	vec4 moltenHeightC = vec4(heightDataC.y);
	vec4 dirtHeightC   = vec4(heightDataC.z);
	vec4 waterHeightC  = vec4(heightDataC.w);
	
	vec4 rockHeightN   = vec4(heightDataL.x, heightDataR.x, heightDataU.x, heightDataD.x);
	vec4 moltenHeightN = vec4(heightDataL.y, heightDataR.y, heightDataU.y, heightDataD.y);
	vec4 dirtHeightN   = vec4(heightDataL.z, heightDataR.z, heightDataU.z, heightDataD.z);
	vec4 waterHeightN  = vec4(heightDataL.w, heightDataR.w, heightDataU.w, heightDataD.w);

	// Water flux
	{
		vec4 heightC = rockHeightC + dirtHeightC + moltenHeightC + waterHeightC;
		vec4 heightN = rockHeightN + dirtHeightN + moltenHeightN + waterHeightN;
		vec4 heightDiff = max( heightC - heightN, vec4(0.0f) );

		vec4 waterFluxC = texelFetch(s_waterFluxData, texelCoordC, 0);
		waterFluxC += heightDiff;

		// Need to scale down the new flux so that we can't drain more fluid than we have this step
		float totalOutflow = waterFluxC.x + waterFluxC.y + waterFluxC.z + waterFluxC.w + 0.000001;
		float limit = min(1.0f, waterHeightC.x / totalOutflow );
		waterFluxC *= limit;
		waterFluxC *= u_waterFluxDamping;

		out_waterFluxData = waterFluxC;
	}
	
}








