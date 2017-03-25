#version 330 core

// Inputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_uv;
layout(location = 3) in vec4 in_color;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	vec3 out_viewNormal;
	vec4 out_viewPosition;
	vec4 out_color;
};

void main(void)
{
	out_viewNormal = normalize(in_normal * u_normalModelViewMatrix);
	out_viewPosition = u_modelViewMatrix * vec4(in_position, 1);
	out_color = in_color;

	gl_Position = u_mvpMatrix * vec4(in_position, 1);
} 