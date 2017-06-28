#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_pressureData;
uniform sampler2D s_divergenceData;

uniform float u_alpha;			// -cellSize * cellSize
uniform float u_invBeta = 0.25;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring pressure:
    vec4 pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1));
    vec4 pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1));
    vec4 pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0));
    vec4 pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0));
    vec4 pC = texelFetch(s_pressureData, T, 0);

    vec4 bC = texelFetch(s_divergenceData, T, 0);
    out_fragColor = (pW + pE + pS + pN + u_alpha * bC) * u_invBeta;
}