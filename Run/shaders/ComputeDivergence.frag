#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_fluxData;
uniform sampler2D s_heightData;
uniform float u_halfInverseCellSize; // 0.5f / CellSize

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);
	
	// Get molten heights
	vec4 hC = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  0));
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
    vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
    vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
    vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));

    // Find neighboring velocities:
    vec4 fN = texelFetchOffset(s_fluxData, T, 0, ivec2( 0, -1)) * hC.y;
    vec4 fS = texelFetchOffset(s_fluxData, T, 0, ivec2( 0,  1)) * hC.y;
    vec4 fE = texelFetchOffset(s_fluxData, T, 0, ivec2( 1,  0)) * hC.y;
    vec4 fW = texelFetchOffset(s_fluxData, T, 0, ivec2(-1,  0)) * hC.y;
	
	vec2 vN = vec2(fN.y - fN.x, fN.w - fN.z);
	vec2 vS = vec2(fS.y - fS.x, fS.w - fS.z);
	vec2 vE = vec2(fE.y - fE.x, fE.w - fE.z);
	vec2 vW = vec2(fW.y - fW.x, fW.w - fW.z);

	float divergenceA = u_halfInverseCellSize * (vE.x-vW.x + vS.y-vN.y);
	
    out_fragColor = vec4(divergenceA, 0.0, 0.0, 0.0);
}