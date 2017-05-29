#version 330 core

// Inputs
in Varying
{
	vec4 in_viewPosition;
};

uniform samplerCube s_envMap;

uniform float u_mipLevel;
uniform float u_intensity;

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

void main(void)
{ 
	vec4 envMapSample = textureLod( s_envMap, normalize(in_viewPosition.xyz), u_mipLevel );

	out_fragColor = envMapSample * u_intensity;
}