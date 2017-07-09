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

const float EPSILON = 0.0000001;

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	float dt = u_dt * 100.0;
	
	vec4 hC = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  0));
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));

    vec2 vC = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  0)).xy * dt;
	vec2 vN = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1)).xy * dt;
	vec2 vS = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1)).xy * dt;
	vec2 vE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0)).xy * dt;
	vec2 vW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0)).xy * dt;
	
	float slopeGradientStrength = 0.5;
	float slopeN = (hN.x+hN.y - hC.x+hC.y) * slopeGradientStrength;
	float slopeS = (hC.x+hC.y - hS.x+hS.y) * slopeGradientStrength;
	float slopeE = (hC.x+hC.y - hE.x+hE.y) * slopeGradientStrength;
	float slopeW = (hW.x+hW.y - hC.x+hC.y) * slopeGradientStrength;
	
	float vCN = clamp(-vC.y + slopeN, 0.0, 1.0);
	float vCS = clamp( vC.y + slopeS, 0.0, 1.0);
	float vCE = clamp( vC.x + slopeE, 0.0, 1.0);
	float vCW = clamp(-vC.x + slopeW, 0.0, 1.0);
	
	float vNC = clamp( vN.y + slopeN, 0.0, 1.0);
	float vSC = clamp(-vS.y + slopeS, 0.0, 1.0);
	float vEC = clamp(-vE.x + slopeE, 0.0, 1.0);
	float vWC = clamp( vW.x + slopeW, 0.0, 1.0);

	float toN = min( vCN * hC.y, hC.y * 0.25 );
	float toS = min( vCS * hC.y, hC.y * 0.25 );
	float toE = min( vCE * hC.y, hC.y * 0.25 );
	float toW = min( vCW * hC.y, hC.y * 0.25 );
	float totalTo = (toN + toS + toE + toW);
	
	float fromN = min( vNC * hN.y, hN.y * 0.25 );
	float fromS = min( vSC * hS.y, hS.y * 0.25 );
	float fromE = min( vEC * hE.y, hE.y * 0.25 );
	float fromW = min( vWC * hW.y, hW.y * 0.25 );
	
	float totalFrom = fromN + fromS + fromE + fromW;
	
	float moltenHeight = hC.y;
	moltenHeight += totalFrom;
	moltenHeight -= totalTo;
	
	// Advect heat
	vec4 miscDataC = texelFetchOffset(s_miscData, T, 0, ivec2( 0,  0));
	float heatC = miscDataC.x;
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
	
	 out_heightData = vec4( hC.x, moltenHeight, hC.z, hC.w );
	 out_miscData = vec4( heatC, miscDataC.yzw );
}