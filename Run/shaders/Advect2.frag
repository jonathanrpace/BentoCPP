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

	float toN = min( clamp(-vC.y, 0.0, 1.0) * hC.y, hC.y * 0.25 );
	float toS = min( clamp( vC.y, 0.0, 1.0) * hC.y, hC.y * 0.25 );
	float toE = min( clamp( vC.x, 0.0, 1.0) * hC.y, hC.y * 0.25 );
	float toW = min( clamp(-vC.x, 0.0, 1.0) * hC.y, hC.y * 0.25 );

	float fromN = min( clamp( vN.y, 0.0, 1.0) * hN.y, hN.y * 0.25 );
	float fromS = min( clamp(-vS.y, 0.0, 1.0) * hS.y, hS.y * 0.25 );
	float fromE = min( clamp(-vE.x, 0.0, 1.0) * hE.y, hE.y * 0.25 );
	float fromW = min( clamp( vW.x, 0.0, 1.0) * hW.y, hW.y * 0.25 );

	float moltenHeight = hC.y;

	moltenHeight += (fromN + fromS + fromE + fromW);
	moltenHeight -= (toN + toS + toE + toW);

	float avg = (hC.y + hN.y + hS.y + hE.y + hW.y) * 0.2;
	moltenHeight = mix( moltenHeight, avg, 0.04 );

    out_heightData = vec4( hC.x, moltenHeight, hC.z, hC.w );

	TODO Advect heat
}