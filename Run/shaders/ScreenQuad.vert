#version 330 core

// Inputs
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	out vec2 out_uv;
};

void main(void)
{
	gl_Position = vec4( in_position, 0.0, 1.0 );

	out_uv = in_uv;
}