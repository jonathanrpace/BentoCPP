#version 330 core

// Inputs
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	out vec3 out_normal;
	out vec2 out_uv;
};

// Uniforms
uniform mat4 mvpMatrix;

void main(void)
{
	vec4 screenPos = vec4(in_position, 1.0) * mvpMatrix;

	gl_Position = screenPos;
	out_normal = in_normal;
	out_uv = in_uv;
}