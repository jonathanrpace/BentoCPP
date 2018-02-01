#version 430 core

const vec4 EPSILON = vec4(0.000001f);
const float PI = 3.14159265359;
const float HALF_PI = PI * 0.5f;

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_moltenFluxData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_miscData;

uniform float u_moltenDiffuseStrength;

const float DT = 1.0 / 6.0;

// Outputs
layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_miscData;

////////////////////////////////////////////////////////////////
//
void main(void)
{ 
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 hC = texelFetch(s_heightData, T, 0);
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0,-1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0, 1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1, 0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1, 0));
	vec4 miscC = texelFetch(s_miscData, T, 0);
	
	float solidHeight = hC.x;
	float moltenHeight = hC.y;
	float dirtHeight = hC.z;
	float waterHeight = hC.w;
	
	// Diffuse molten volume between cells using a volume preserving blur
	{
		vec4 heightC = vec4( solidHeight + moltenHeight );
		vec4 heightN = vec4( hW.x + hW.y, hE.x + hE.y, hN.x + hN.y, hS.x + hS.y );
		vec4 diffs = heightN - heightC;
		
		vec4 clampedDiffs = clamp( diffs * 0.25 * u_moltenDiffuseStrength, -vec4(hC.y * 0.25), vec4(hW.y, hE.y, hN.y, hS.y) * 0.25 );
		moltenHeight += (clampedDiffs.x + clampedDiffs.y + clampedDiffs.z + clampedDiffs.w) * DT;
		moltenHeight = max(0.0, moltenHeight);
	}
	
	// Update molten heights and heat based on flux
	{
		vec4 fluxC = texelFetch(s_moltenFluxData, T, 0);
		vec4 fluxN = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 0,-1));
		vec4 fluxS = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 0, 1));
		vec4 fluxE = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 1, 0));
		vec4 fluxW = texelFetchOffset(s_moltenFluxData, T, 0, ivec2(-1, 0));
		
		float toN = min( fluxC.z, hC.y ) * DT;
		float toS = min( fluxC.w, hC.y ) * DT;
		float toE = min( fluxC.y, hC.y ) * DT;
		float toW = min( fluxC.x, hC.y ) * DT;
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( fluxN.w, hN.y ) * DT;
		float fromS = min( fluxS.z, hS.y ) * DT;
		float fromE = min( fluxE.x, hE.y ) * DT;
		float fromW = min( fluxW.y, hW.y ) * DT;

		float totalFrom = fromN + fromS + fromE + fromW;
		
		moltenHeight += totalFrom;
		moltenHeight -= totalTo;
	
		float advectSpeed = 0.99;
		if ( moltenHeight > 0 )
		{
			// Advect heat and molten scalar
			vec2 heatC = miscC.xy;
			vec2 heatN = texelFetchOffset(s_miscData, T, 0, ivec2( 0,-1)).xy;
			vec2 heatS = texelFetchOffset(s_miscData, T, 0, ivec2( 0, 1)).xy;
			vec2 heatE = texelFetchOffset(s_miscData, T, 0, ivec2( 1, 0)).xy;
			vec2 heatW = texelFetchOffset(s_miscData, T, 0, ivec2(-1, 0)).xy;
		
			float propC = hC.y > 0.002 ? min( 1.0, totalTo / hC.y ) : 0.0;
			miscC.xy -= propC * heatC * advectSpeed;
			
			float propN = hN.y > 0.002 ? min( 1.0, fromN / hN.y ) : 0.0;
			float propS = hS.y > 0.002 ? min( 1.0, fromS / hS.y ) : 0.0;
			float propE = hE.y > 0.002 ? min( 1.0, fromE / hE.y ) : 0.0;
			float propW = hW.y > 0.002 ? min( 1.0, fromW / hW.y ) : 0.0;

			miscC.xy += ((propN * heatN) + (propS * heatS) + (propE * heatE) + (propW * heatW)) * advectSpeed;
		}
	}
	
	// Update water height based on water flux
	{
		vec4 waterFluxC = texelFetch(s_waterFluxData, T, 0);
		vec4 waterFluxN = texelFetchOffset(s_waterFluxData, T, 0, ivec2( 0,-1));
		vec4 waterFluxS = texelFetchOffset(s_waterFluxData, T, 0, ivec2( 0, 1));
		vec4 waterFluxE = texelFetchOffset(s_waterFluxData, T, 0, ivec2( 1, 0));
		vec4 waterFluxW = texelFetchOffset(s_waterFluxData, T, 0, ivec2(-1, 0));
		
		float toN = min( waterFluxC.z, hC.w ) * DT;
		float toS = min( waterFluxC.w, hC.w ) * DT;
		float toE = min( waterFluxC.y, hC.w ) * DT;
		float toW = min( waterFluxC.x, hC.w ) * DT;
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( waterFluxN.w, hN.w ) * DT;
		float fromS = min( waterFluxS.z, hS.w ) * DT;
		float fromE = min( waterFluxE.x, hE.w ) * DT;
		float fromW = min( waterFluxW.y, hW.w ) * DT;

		float totalFrom = fromN + fromS + fromE + fromW;
		
		waterHeight += totalFrom;
		waterHeight -= totalTo;
	}
	
	out_heightData = max( vec4(0.0), vec4( solidHeight, moltenHeight, dirtHeight, waterHeight ) );
	out_miscData = miscC;
}