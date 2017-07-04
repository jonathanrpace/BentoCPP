#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

layout( location = 0 ) out vec4 out_heightData;

uniform sampler2D s_velocityTexture;
uniform sampler2D s_heightData;

uniform float u_dt;

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 hC = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  0));
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));

    vec2 vC = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  0)).xy * u_dt;
	vec2 vN = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1)).xy * u_dt;
	vec2 vS = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1)).xy * u_dt;
	vec2 vE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0)).xy * u_dt;
	vec2 vW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0)).xy * u_dt;

	float toN = clamp(-vC.y, 0.0, 1.0) * hC.y * 0.25;
	float toS = clamp( vC.y, 0.0, 1.0) * hC.y * 0.25;
	float toE = clamp( vC.x, 0.0, 1.0) * hC.y * 0.25;
	float toW = clamp(-vC.x, 0.0, 1.0) * hC.y * 0.25;

	float fromN = clamp( vN.y, 0.0, 1.0) * hN.y * 0.25;
	float fromS = clamp(-vS.y, 0.0, 1.0) * hS.y * 0.25;
	float fromE = clamp(-vE.x, 0.0, 1.0) * hE.y * 0.25;
	float fromW = clamp( vW.x, 0.0, 1.0) * hW.y * 0.25;

	float moltenHeight = hC.y;

	moltenHeight += (fromN + fromS + fromE + fromW);
	moltenHeight -= (toN + toS + toE + toW);

    out_heightData = vec4( hC.x, moltenHeight, hC.z, hC.w );
}