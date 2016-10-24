#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

// Samplers
uniform sampler2D s_texture;

in Varying
{
	float in_color;
	vec2 in_uv;
};

void main(void)
{
	//float textureSample = texture( s_texture, in_uv ).x;
	float color = in_color;// * (1.0-textureSample);
	out_fragColor = vec4(color, dFdx(color), dFdy(color), 0.0);
} 