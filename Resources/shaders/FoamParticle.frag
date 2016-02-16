#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in Varying
{
	float in_alpha;
};

void main(void)
{
	out_fragColor = vec4(1.0, 1.0, 1.0, in_alpha);
} 