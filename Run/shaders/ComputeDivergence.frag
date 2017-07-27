#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform float u_halfInverseCellSize; // 0.5f / CellSize

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring velocities:
    vec4 fN = texelFetchOffset(s_velocityData, T, 0, ivec2( 0, -1));
    vec4 fS = texelFetchOffset(s_velocityData, T, 0, ivec2( 0,  1));
    vec4 fE = texelFetchOffset(s_velocityData, T, 0, ivec2( 1,  0));
    vec4 fW = texelFetchOffset(s_velocityData, T, 0, ivec2(-1,  0));
	
	vec2 vN = vec2(fN.y - fN.x, fN.w - fN.z);
	vec2 vS = vec2(fS.y - fS.x, fS.w - fS.z);
	vec2 vE = vec2(fE.y - fE.x, fE.w - fE.z);
	vec2 vW = vec2(fW.y - fW.x, fW.w - fW.z);

	// Falloff divergence to zero as volume becomes very small
	// Stops seemining invisible pressure pushing non-local volume away.
	vec4 hC = texelFetch(s_heightData, T, 0);
	float volumeScalar = smoothstep( 0.0, 0.001, hC.y );
	
	float divergenceA = u_halfInverseCellSize * (vE.x-vW.x + vS.y-vN.y) * volumeScalar;
	
    out_fragColor = vec4(divergenceA, 0.0, 0.0, 0.0);
}