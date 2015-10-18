#version 150 core

// Inputs
in Varying
{
	vec3 in_normal;
	vec2 in_uv;
};

// Outputs
out vec4 color;

void main(void)
{
	color = vec4(in_uv,0.0,1.0);
}