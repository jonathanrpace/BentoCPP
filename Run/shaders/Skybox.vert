#version 330 core

// Inputs
layout(location = 0) in vec3 in_position;

// Uniforms
uniform mat3 u_normalViewMat;
uniform mat4 u_projMat;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	out vec4 out_viewPosition;
};

void main(void)
{
	gl_Position = u_projMat * vec4( in_position, 1.0 );
	out_viewPosition = vec4( in_position * u_normalViewMat, 1.0 );
}