#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

out vec4 out_fragColor;

uniform sampler2D s_velocityTexture;
uniform sampler2D s_sourceTexture;

uniform float u_dt;
uniform float u_dissipation;

void main()
{
	float cellScalar = 1.0 / textureSize(s_velocityTexture,0).x;
	
	vec4 fluxSample = texelFetch( s_velocityTexture, ivec2(gl_FragCoord.xy), 0 );
    vec2 velocity = vec2(fluxSample.y - fluxSample.x, fluxSample.w - fluxSample.z);
	
    vec2 coord = in_uv - velocity * u_dt * cellScalar;
	
	vec4 outFragColor = max( texture(s_sourceTexture, coord), vec4(0.0) );

    out_fragColor = outFragColor;
}