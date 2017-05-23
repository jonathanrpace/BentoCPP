#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec3 in_worldPosition;
	vec4 in_viewPosition;
	vec2 in_uv;
};

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_worldNormal;
layout( location = 2 ) out vec4 out_forward;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////
void main(void)
{
	out_worldNormal = vec4(0.0);
	out_viewPosition = in_viewPosition;
	out_forward = vec4( 1.0, 0.0, 0.0, 1.0 );
}
