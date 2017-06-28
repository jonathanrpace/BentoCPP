#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_velocityData;
uniform float u_halfInverseCellSize; // 0.5f / CellSize

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring velocities:
    vec4 vN = texelFetchOffset(s_velocityData, T, 0, ivec2( 0, -1));
    vec4 vS = texelFetchOffset(s_velocityData, T, 0, ivec2( 0,  1));
    vec4 vE = texelFetchOffset(s_velocityData, T, 0, ivec2( 1,  0));
    vec4 vW = texelFetchOffset(s_velocityData, T, 0, ivec2(-1,  0));
	
	float divergenceA = u_halfInverseCellSize * (vE.x-vW.x + vS.y-vN.y);
	float divergenceB = u_halfInverseCellSize * (vE.z-vW.z + vS.w-vN.w);
	
    out_fragColor = vec4(divergenceA, divergenceB, 0.0, 0.0);
}