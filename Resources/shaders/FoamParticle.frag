#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in Varying
{
	vec4 in_color;
};

void main(void)
{
	vec2 temp = gl_PointCoord - vec2(0.5);
	float dp = dot(temp,temp);

	float f = 1.0-(sqrt(dp) / 0.5);

	out_fragColor = vec4(in_color.xyz, f * in_color.w);
} 