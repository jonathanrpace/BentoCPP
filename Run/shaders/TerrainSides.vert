#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_uv;

// Samplers
uniform sampler2D s_heightData;

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
	vec3 out_uv;
	vec4 out_heightData;
};

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	vec4 position = vec4(in_position, 1.0f);
	vec4 viewPosition = u_modelViewMatrix * position;

	vec4 heightDataC = texture( s_heightData, in_uv.xy );
	out_heightData = heightDataC;

	float rockHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;

	position.y += (rockHeight + moltenHeight + dirtHeight + waterHeight) * in_uv.z;

	// Output
	{
		out_worldPosition = position.xyz;
		out_viewPosition = viewPosition;
		out_uv = in_uv;
		gl_Position = u_mvpMatrix * position;
	}
} 