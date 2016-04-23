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
uniform float u_heatViscosityBias;
uniform ivec2 u_axis;

// Outputs
layout( location = 0 ) out vec4 out_rockFluxData;
layout( location = 1 ) out vec4 out_waterFluxData;

void main(void)
{ 
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,1) * u_axis;
	ivec2 texelCoordR = texelCoordC + ivec2(1,1) * u_axis;

	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
	vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);

	vec2 rockHeightC = vec2(rockDataC.x);
	vec2 dirtHeightC = vec2(rockDataC.w);
	vec2 moltenHeightC = vec2(rockDataC.y);
	
	vec2 rockHeightN = vec2(rockDataL.x, rockDataR.x);
	vec2 dirtHeightN = vec2(rockDataL.w, rockDataR.w);
	vec2 moltenHeightN = vec2(rockDataL.y, rockDataR.y);
	
	// Molten flux
	{
		float rockHeatC = rockDataC.z;

		vec4 fluxC = texelFetch(s_rockFluxData, texelCoordC, 0);

		vec2 heightC = rockHeightC + dirtHeightC + moltenHeightC;
		vec2 heightN = rockHeightN + dirtHeightN + moltenHeightN;
		vec2 heightDiff = max( heightC - heightN, vec2(0.0f) );

		if ( u_axis == ivec2(1,0) )
		{
			fluxC.xy += heightDiff;
		}
		else
		{
			fluxC.zw += heightDiff;
		}
		
		// Need to scale down the new flux so that we can't drain more fluid than we have this step
		float totalOutflow = fluxC.x + fluxC.y + fluxC.z + fluxC.w + 0.0001f;
		float limit = min(1.0f, moltenHeightC.x / totalOutflow );
		limit = smoothstep(0,1,limit);
		fluxC *= limit;
		fluxC *= u_rockFluxDamping * step( u_heatViscosityBias, rockHeatC );

		out_rockFluxData = fluxC;
	}

	// Water flux
	{
		vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
		vec4 waterDataL = texelFetch(s_waterData, texelCoordL, 0);
		vec4 waterDataR = texelFetch(s_waterData, texelCoordR, 0);
		vec2 waterHeightC = vec2(waterDataC.x);
		vec2 waterHeightN = vec2(waterDataL.x, waterDataR.x);

		vec4 fluxC = texelFetch(s_waterFluxData, texelCoordC, 0);

		vec2 heightC = rockHeightC + dirtHeightC + moltenHeightC + waterHeightC;
		vec2 heightN = rockHeightN + dirtHeightN + moltenHeightN + waterHeightN;
		vec2 heightDiff = max( heightC - heightN, vec2(0.0f) );

		float totalOutflow = 0.0001;
		if ( u_axis == ivec2(1,0) )
		{
			fluxC.xy += heightDiff;
			totalOutflow += fluxC.x + fluxC.y;
		}
		else
		{
			fluxC.zw += heightDiff;
			totalOutflow += fluxC.z + fluxC.w;
		}
		
		// Need to scale down the new flux so that we can't drain more fluid than we have this step
		float limit = min(1.0f, waterHeightC.x*0.5 / totalOutflow );
		limit = smoothstep(0,1,limit);
		fluxC *= limit;
		fluxC *= u_waterFluxDamping;

		out_waterFluxData = fluxC;
	}
}








