#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};


layout( location = 0 ) out vec4 out_fluidFlux;

uniform sampler2D s_fluidFluxData;

uniform float u_dt;

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	float cellScalar = 1.0 / textureSize(s_fluidFluxData,0).x;
	
	vec4 fluxSample = texelFetch( s_fluidFluxData, T, 0 );
    vec2 velocity = vec2(fluxSample.y - fluxSample.x, fluxSample.w - fluxSample.z);
    vec2 coord = in_uv - velocity * u_dt * cellScalar;

	out_fluidFlux = max( texture(s_fluidFluxData, coord), vec4(0.0) );
}