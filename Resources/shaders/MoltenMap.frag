#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in Varying
{
	float in_color;
};

void main(void)
{
	float color = in_color;
	out_fragColor = vec4(color, dFdx(color), dFdy(color), 0.0);
} 