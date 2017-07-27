#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};


layout( location = 0 ) out vec4 out_fluidFlux;
layout( location = 1 ) out vec4 out_miscData;

uniform sampler2D s_fluidFluxData;
uniform sampler2D s_miscData;

uniform float u_dt;

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	float cellScalar = 1.0 / textureSize(s_fluidFluxData,0).x;
	
	vec4 fluxSample = texelFetch( s_fluidFluxData, ivec2(gl_FragCoord.xy), 0 );
    vec2 velocity = vec2(fluxSample.y - fluxSample.x, fluxSample.w - fluxSample.z);
    vec2 coord = in_uv - velocity * u_dt * cellScalar;

	// Advect flux
	vec4 out_fluidFlux = max( texture(s_fluidFluxData, coord), vec4(0.0) );

	// Advect only the molten scalar 
	vec4 outMiscData = texelFetch( s_miscData, T, 0 );
	//outMiscData.y = texture(s_miscData, coord).y;

    out_fluidFlux = out_fluidFlux;
	out_miscData = outMiscData;
}