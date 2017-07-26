#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_miscData;

uniform sampler2D s_velocityTexture;
uniform sampler2D s_heightData;
uniform sampler2D s_miscData;

uniform float u_dt;
uniform float u_moltenDiffusionStrength;
uniform float u_waterDiffusionStrength = 1.0;

const float EPSILON = 0.0000001;

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	float dt = u_dt * 100.0;
	
	vec4 miscDataC = texelFetchOffset(s_miscData, T, 0, ivec2( 0,  0));
	float heatC = miscDataC.x;
	
	vec4 hC = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  0));
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));
	
	//float moltenHeightC = hC.x + hC.y;
	//float moltenHeightN = hN.x + hN.y;
	//float moltenHeightS = hS.x + hS.y;
	//float moltenHeightE = hE.x + hE.y;
	//float moltenHeightW = hW.x + hW.y;
	
	//vec2 fluidDiffusionStrength = vec2(u_moltenDiffusionStrength, u_waterDiffusionStrength);
	//vec2 slopeN = (fluidHeightN - fluidHeightC) * fluidDiffusionStrength;
	//vec2 slopeS = (fluidHeightS - fluidHeightC) * fluidDiffusionStrength;
	//vec2 slopeE = (fluidHeightE - fluidHeightC) * fluidDiffusionStrength;
	//vec2 slopeW = (fluidHeightW - fluidHeightC) * fluidDiffusionStrength;
	
	vec4 fC = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  0));
	vec4 fN = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1));
	vec4 fS = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1));
	vec4 fE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0));
	vec4 fW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0));
	
	/*
	vec4 vC = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  0)) * dt;
	vec4 vN = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1)) * dt;
	vec4 vS = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1)) * dt;
	vec4 vE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0)) * dt;
	vec4 vW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0)) * dt;
	
	vec2 vCN = clamp(-vec2(vC.y, vC.w) - slopeN, vec2(0.0), vec2(1.0));
	vec2 vCS = clamp( vec2(vC.y, vC.w) - slopeS, vec2(0.0), vec2(1.0));
	vec2 vCE = clamp( vec2(vC.x, vC.z) - slopeE, vec2(0.0), vec2(1.0));
	vec2 vCW = clamp(-vec2(vC.x, vC.z) - slopeW, vec2(0.0), vec2(1.0));
	
	vec2 vNC = clamp( vec2(vN.y, vN.w) + slopeN, vec2(0.0), vec2(1.0));
	vec2 vSC = clamp(-vec2(vS.y, vS.w) + slopeS, vec2(0.0), vec2(1.0));
	vec2 vEC = clamp(-vec2(vE.x, vE.z) + slopeE, vec2(0.0), vec2(1.0));
	vec2 vWC = clamp( vec2(vW.x, vW.z) + slopeW, vec2(0.0), vec2(1.0));
	*/
	
	// Transfer molten volume
	float moltenHeight = hC.y;
	{
		float toN = min( fC.z * hC.y, hC.y * 0.15 );
		float toS = min( fC.w * hC.y, hC.y * 0.15 );
		float toE = min( fC.y * hC.y, hC.y * 0.15 );
		float toW = min( fC.x * hC.y, hC.y * 0.15 );
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( fN.w * hN.y, hN.y * 0.15 );
		float fromS = min( fS.z * hS.y, hS.y * 0.15 );
		float fromE = min( fE.x * hE.y, hE.y * 0.15 );
		float fromW = min( fW.y * hW.y, hW.y * 0.15 );

		float totalFrom = fromN + fromS + fromE + fromW;

		moltenHeight += totalFrom;
		moltenHeight -= totalTo;
		
		// Advect heat
		/*
		float heatN = texelFetchOffset(s_miscData, T, 0, ivec2( 0, -1)).x;
		float heatS = texelFetchOffset(s_miscData, T, 0, ivec2( 0,  1)).x;
		float heatE = texelFetchOffset(s_miscData, T, 0, ivec2( 1,  0)).x;
		float heatW = texelFetchOffset(s_miscData, T, 0, ivec2(-1,  0)).x;
		
		float toProp = totalTo / max( hC.y, EPSILON );
		float toHeat = toProp * heatC;
		
		vec4 fromProp = vec4(fromN, fromS, fromE, fromW) / max( vec4( hN.y, hS.y, hE.y, hW.y ), vec4( EPSILON ) );
		float fromheat = dot( vec4( heatN, heatS, heatE, heatW ) * fromProp, vec4(1.0) );
		
		heatC -= toHeat;
		heatC += fromheat;
		*/
	}
		
	out_heightData = vec4( hC.x, moltenHeight, hC.z, hC.w );
	out_miscData = vec4( heatC, miscDataC.yzw );
}