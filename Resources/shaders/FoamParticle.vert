#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform float u_terrainSize = 1.5;
uniform vec3 u_lightDir = vec3(0.0,1.0,0.0);

uniform sampler2D s_waterData; 
uniform sampler2D s_rockData; 

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
};

out Varying
{
	vec4 out_color;
};

void main(void)
{
	float life = in_velocity.w;

	vec4 position = in_position;
	position.xz *= u_terrainSize;
	position.xz -= u_terrainSize*0.5;
	position.y += 0.01;

	vec4 screenPos = u_mvpMatrix * position;

	vec3 normal = in_velocity.xyz;
	normal.y = 1;
	normal = normalize(normal);
	float lightDot = dot(normal, u_lightDir);

	gl_PointSize = 4.0f / (1.0f+screenPos.z);
	gl_Position = screenPos;

	out_color = vec4(lightDot, lightDot, lightDot, life);
} 