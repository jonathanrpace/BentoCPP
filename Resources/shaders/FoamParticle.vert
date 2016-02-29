#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;
layout(location = 3) in vec3 in_normal;

// Uniforms
uniform float u_terrainSize = 1.5;

uniform sampler2D s_waterData; 
uniform sampler2D s_rockData; 

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	vec4 out_color;
};

void main(void)
{
	vec4 screenPos = vec4(in_position.x, in_position.z, 1.0, 1.0);
	screenPos.xy -= 0.5;
	screenPos.xy *= 2.0;
	gl_Position = screenPos;

	float life = in_velocity.w;
	out_color = vec4(life * 0.2);
} 