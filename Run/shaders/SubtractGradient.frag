#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_velocityData;
uniform sampler2D s_pressureData;

uniform float u_gradientScale;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

	vec4 outFlux = texelFetch(s_velocityData, T, 0);
	
    // Subtract pressure gradient
	if ( true )
	{
		float pC = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  0)).x;
		float pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1)).x;
		float pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1)).x;
		float pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0)).x;
		float pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0)).x;
	
		//vec2 pressureGradient = vec2(pE.x - pW.x, pS.x - pN.x);
		
		// TODO - Don't subtract from areas with no volume
		
		outFlux.x += (pC - pW) * u_gradientScale * 100.0;
		outFlux.y += (pC - pE) * u_gradientScale * 100.0;
		outFlux.z += (pC - pN) * u_gradientScale * 100.0;
		outFlux.w += (pC - pS) * u_gradientScale * 100.0;

		outFlux = clamp( outFlux, vec4(0.0), vec4(1.0));
		
		//newMoltenVelocity -= pressureGradientMolten  * u_gradientScale;
		//newWaterVelocity -= pressureGradientWater  * u_gradientScale;
	}
	
   // out_fragColor = vec4(newMoltenVelocity, newWaterVelocity);
    out_fragColor = outFlux;
}
