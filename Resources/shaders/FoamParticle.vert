#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform float u_terrainSize = 1.5;

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
	vec4 position = in_position;
	position.xz *= u_terrainSize;
	position.xz -= u_terrainSize*0.5;
	position.w = 1.0;

	vec4 screenPos = u_mvpMatrix * position;

	gl_Position = screenPos;

	/*
	vec4 screenPos = vec4(in_position.x, in_position.z, 1.0, 1.0);
	screenPos.xy -= 0.5;
	screenPos.xy *= 2.0;
	gl_Position = screenPos;
	*/

	float life = in_velocity.w;
	float lifeAlpha = sin(life*3.142);
	float waterAlpha = in_position.w;

	out_color = vec4(vec3(1.0), lifeAlpha * waterAlpha);
} 