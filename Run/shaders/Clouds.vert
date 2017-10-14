#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_uv;

// Uniforms
uniform mat4 u_invViewProjMatrix;


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// Varying
out Varying
{
	vec4 out_rayNear;
	vec4 out_rayFar;
};

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	vec2 modifiedUV = (in_uv * 2.0) - 1.0;
	
	vec4 rayNear = vec4( modifiedUV, -1.0, 1.0 );
	vec4 rayFar = vec4( modifiedUV, 1.0, 1.0 );
	
	rayNear = u_invViewProjMatrix * rayNear;
	rayFar = u_invViewProjMatrix * rayFar;
	
	rayNear.xyz /= rayNear.w;
	rayFar.xyz /= rayFar.w;
	
	out_rayNear = rayNear;
	out_rayFar = rayFar;
	
	gl_Position = vec4( in_position, 0.0, 1.0 );
} 