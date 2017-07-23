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

	vec4 oldVelocity = texelFetch(s_velocityData, T, 0);
	vec2 newMoltenVelocity = oldVelocity.xy;
	vec2 newWaterVelocity = oldVelocity.zw;

    // Subtract pressure gradient
	{
		vec4 pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1));
		vec4 pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1));
		vec4 pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0));
		vec4 pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0));
	
		vec2 pressureGradientMolten = vec2(pE.x - pW.x, pS.x - pN.x);
		vec2 pressureGradientWater = vec2(pE.y - pW.y, pS.y - pN.y);

		newMoltenVelocity -= pressureGradientMolten  * u_gradientScale;
		newWaterVelocity -= pressureGradientWater  * u_gradientScale;
	}
	
    out_fragColor = vec4(newMoltenVelocity, newWaterVelocity);
}
