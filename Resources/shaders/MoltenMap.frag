#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

// Samplers

uniform sampler2D s_texture;

in Varying
{
	vec4 in_color;
	float in_angle;
};

void main(void)
{
	vec2 uv = gl_PointCoord - vec2(0.5);
	vec2 rotatedUV = vec2(  uv.x * cos(in_angle) - uv.y * sin(in_angle),
							uv.x * sin(in_angle) + uv.y * cos(in_angle) );
	rotatedUV += vec2(0.5);

	vec4 textureSample = texture(s_texture, rotatedUV);
	float alpha = textureSample.x;
	alpha *= in_color.w;

	out_fragColor = vec4(in_color.xyz*alpha, alpha);
} 