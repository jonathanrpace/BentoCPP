#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_velocityData;
uniform sampler2D s_pressureData;
uniform float u_gradientScale = 100.0;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring pressure:
    vec4 pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1));
    vec4 pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1));
    vec4 pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0));
    vec4 pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0));
	
    vec2 gradA = vec2(pE.x - pW.x, pS.x - pN.x);
    vec2 gradB = vec2(pE.y - pW.y, pS.y - pN.y);
	
	vec2 oldV = texelFetch(s_velocityData, T, 0).xy;
	
    vec2 newVA = oldV - gradA  * u_gradientScale;
    vec2 newVB = oldV - gradB  * u_gradientScale;
    out_fragColor = vec4(newVA, newVB);
}
