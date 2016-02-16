#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform sampler2D s_waterNormalData;

// Outputs
out Varying
{
	vec4 out_position;
	vec3 out_velocity;
};

void main(void)
{
	vec4 position = in_position;
	position.y += 0.01f;

	out_position = position;
	out_velocity = in_velocity;
} 