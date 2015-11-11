#version 330 core

// Samplers
uniform sampler2D s_heightData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform vec2 u_axis;
uniform float u_strength;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void main(void)
{ 
	ivec2 dimensions = textureSize(s_heightData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 height = texture(s_heightData, in_uv);
	vec4 heightA = texture(s_heightData, in_uv - u_axis * texelSize);
	vec4 heightB = texture(s_heightData, in_uv + u_axis * texelSize);

	float newHeight = height.y;
	newHeight += clamp((heightA.y-height.y), -height.y*0.5, heightA.y*0.5) * u_strength;
	newHeight += clamp((heightB.y-height.y), -height.y*0.5, heightB.y*0.5) * u_strength;

	out_heightData = vec4(height.x, newHeight, height.zw);
}








