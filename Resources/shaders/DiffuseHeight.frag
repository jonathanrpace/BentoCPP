#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform vec2 u_axis;
uniform float u_strength;
uniform float u_heatDissipation;
uniform float u_heatViscosity;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void main(void)
{ 
	ivec2 dimensions = textureSize(s_heightData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 height = texture(s_heightData, in_uv);
	vec4 heightA = texture(s_heightData, in_uv - u_axis * texelSize);
	vec4 heightB = texture(s_heightData, in_uv + u_axis * texelSize);

	vec2 velocity = texture(s_velocityData, in_uv).xy;

	// Smooth height
	float newHeight = height.y;
	newHeight += clamp((heightA.y-height.y), -height.y*0.5, heightA.y*0.5) * u_strength;
	newHeight += clamp((heightB.y-height.y), -height.y*0.5, heightB.y*0.5) * u_strength;

	// Move heat
	float newHeat = height.z;
	vec2 fromA = max(vec2(0.0f), velocity) * u_axis * heightA.z * 0.25f;
	vec2 fromB = max(vec2(0.0f),-velocity) * u_axis * heightB.z * 0.25f;
	newHeat += min(1.0f, (fromA.x + fromA.y + fromB.x + fromB.y) * u_heatViscosity);

	vec2 toA = max(vec2(0.0f),-velocity) * u_axis * newHeat * 0.25f;
	vec2 toB = max(vec2(0.0f), velocity) * u_axis * newHeat * 0.25f;
	newHeat -= min(1.0f, (toA.x+toA.y+toB.x+toB.y) * u_heatViscosity);

	// Smooth heat
	newHeat += clamp((heightA.z-height.z), -height.z*0.5, heightA.z*0.5) * u_heatDissipation;
	newHeat += clamp((heightB.z-height.z), -height.z*0.5, heightB.z*0.5) * u_heatDissipation;
	
	newHeat = clamp(newHeat, 0.0f, 1.0f);
	

	out_heightData = vec4(height.x, newHeight, newHeat, height.w);
}








