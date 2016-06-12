#version 330 core

// Samplers
uniform sampler2DRect s_tex;

// Inputs
in Varying
{
	vec2 in_uv;
};

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

void main(void)
{ 
	ivec2 texSize = textureSize( s_tex, 0 );
	vec2 uv = in_uv;
	uv *= texSize;
	out_fragColor = texture2DRect( s_tex, uv );
}