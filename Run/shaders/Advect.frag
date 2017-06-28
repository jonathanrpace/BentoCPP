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
    vec2 velocity = texture(s_velocityTexture, in_uv).xy;
    vec2 coord = in_uv - velocity * u_dt;
    out_fragColor = u_dissipation * texture(s_sourceTexture, coord);
}