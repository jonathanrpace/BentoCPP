#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;


in Varying
{
	vec4 in_color;
};

void main(void)
{
	out_fragColor = in_color;
} 