#version 330 core

in Varying
{
	vec3 in_position;
	vec4 in_velocity;
};

// Outputs
layout( location = 0 ) out vec4 out_colour;

void main(void)
{ 
	out_colour = vec4(1.0,0.0,0.0,0.0);
}