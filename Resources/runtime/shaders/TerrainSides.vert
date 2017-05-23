#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Uniforms
uniform mat4 u_viewMatrix;
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;

uniform vec3 u_cameraPos;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// Varying
out Varying
{
	vec3 out_worldPosition;
	vec4 out_viewPosition;
	vec2 out_uv;
};

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	vec4 position = vec4(in_position, 1.0f);
	vec4 viewPosition = u_modelViewMatrix * position;

	// Output
	{
		out_worldPosition = position.xyz;
		out_viewPosition = viewPosition;
		out_uv = in_uv;
		gl_Position = u_mvpMatrix * position;
	}
} 