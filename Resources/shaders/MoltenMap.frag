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

	//vec2 delta = gl_PointCoord - vec2(0.5);
	//float len = length(delta) * 2.0;
	//float alpha = (1.0-len);

	vec4 textureSample = texture(s_texture, rotatedUV);
	float alpha = textureSample.x;
	alpha -= (1.0-in_color.w);
	//alpha = pow(alpha, 0.3);

	out_fragColor = vec4(in_color.xyz * alpha, 1.0);
} 