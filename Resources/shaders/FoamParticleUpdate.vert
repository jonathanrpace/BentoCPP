#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in vec4 in_prperties;

// Uniforms

// Outputs
out Varying
{
	vec4 out_position;
	vec3 out_velocity;
};

void main(void)
{
	out_position = in_position;
	out_velocity = in_velocity;
} 